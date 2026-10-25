#include "codexion.h"

static uint32_t left(uint32_t i, uint32_t max)
{
    return (i - 1) + (max * (i == 0));
}

static uint32_t right(uint32_t i, uint32_t max)
{
    return (i + 1) % max;
}


typedef struct assigne_one_s{
    uint32_t    *could_compile;
    sheduler_t  *sh;
} assign_t;

static error_t assigne_one(void **args)
{
    uint32_t    i;
    uint32_t    count;
    assign_t    *vars;

    vars = (assign_t *)args;
    i = 0;
    *vars->could_compile = 0;
    while (i < vars->sh->nb_coders)
    {
        count = 0;
        if (vars->sh->dungles[i] == DUNGLE_UNUSED
            && vars->sh->dungles[left(i, vars->sh->nb_coders)] == DUNGLE_UNUSED)
        {
            // who is waiting ?
            pthread_mutex_lock(vars->sh->can_take_mtx[left(i, vars->sh->nb_coders)]);
            count += *vars->sh->can_take_dungles[left(i, vars->sh->nb_coders)] == WAITING;
            pthread_mutex_unlock(vars->sh->can_take_mtx[left(i, vars->sh->nb_coders)]);
            
            pthread_mutex_lock(vars->sh->can_take_mtx[i]);
            count += (*vars->sh->can_take_dungles[i] == WAITING) * 2;
            pthread_mutex_unlock(vars->sh->can_take_mtx[i]);
            
            pthread_mutex_lock(vars->sh->can_take_mtx[right(i, vars->sh->nb_coders)]);
            count += (*vars->sh->can_take_dungles[right(i, vars->sh->nb_coders)] == WAITING) * 4;
            pthread_mutex_unlock(vars->sh->can_take_mtx[right(i, vars->sh->nb_coders)]);

            // another priority ?
            if (!((count >> 1) & 1))
                continue;
            if (count & 1)
            {
                pthread_mutex_lock(vars->sh->ask_times_mtx[i]);
                pthread_mutex_lock(vars->sh->ask_times_mtx[left(i, vars->sh->nb_coders)]);
                if (*vars->sh->ask_times[i] > *vars->sh->ask_times[left(i, vars->sh->nb_coders)])
                {
                    pthread_mutex_unlock(vars->sh->ask_times_mtx[i]);
                    pthread_mutex_unlock(vars->sh->ask_times_mtx[left(i, vars->sh->nb_coders)]);
                    i++;
                    continue;
                }
                pthread_mutex_unlock(vars->sh->ask_times_mtx[left(i, vars->sh->nb_coders)]);
                pthread_mutex_unlock(vars->sh->ask_times_mtx[i]);
            }
            if ((count >> 2) & 1)
            {
                pthread_mutex_lock(vars->sh->ask_times_mtx[i]);
                pthread_mutex_lock(vars->sh->ask_times_mtx[right(i, vars->sh->nb_coders)]);
                if (*vars->sh->ask_times[i] > *vars->sh->ask_times[right(i, vars->sh->nb_coders)])
                {
                    pthread_mutex_unlock(vars->sh->ask_times_mtx[right(i, vars->sh->nb_coders)]);
                    pthread_mutex_unlock(vars->sh->ask_times_mtx[i]);
                    i++;
                    continue;
                }
                pthread_mutex_unlock(vars->sh->ask_times_mtx[right(i, vars->sh->nb_coders)]);
                pthread_mutex_unlock(vars->sh->ask_times_mtx[i]);
            }
            *vars->could_compile = 1;
            // assigne the dungle
            vars->sh->dungles[i] = i;
            vars->sh->dungles[left(i, vars->sh->nb_coders)] = i;
            pthread_mutex_lock(vars->sh->can_take_mtx[i]);
            *vars->sh->can_take_dungles[i] = CAN_TAKE_DUNGLE;
            pthread_mutex_unlock(vars->sh->can_take_mtx[i]);
            
            pthread_cond_broadcast(vars->sh->can_take_cond[i]);
            return SUCCESS;
        }
        i++;
    }
    return SUCCESS;
}

typedef struct error_free_s {

    sheduler_t  *sheduler;
    uint32_t    i;
    uint32_t    dungle;
    uint64_t    now;
    uint64_t    *next;
}   error_free_t;

static error_t error_free(void **args)
{
    error_free_t    *vars;

    vars = (error_free_t *)args;
    if (pthread_mutex_lock(vars->sheduler->compile_done_times_mtx[vars->i]) != SUCCESS)
        return PTHREAD_ERROR;

    if (vars->now - *vars->sheduler->compile_done_times[vars->i] 
        >= vars->sheduler->dungle_time)
        vars->sheduler->dungles[vars->dungle] = DUNGLE_UNUSED;
    else if (*vars->next > vars->sheduler->dungle_time - vars->now - *vars->sheduler->compile_done_times[vars->i])
        *vars->next = vars->sheduler->dungle_time - (vars->now - *vars->sheduler->compile_done_times[vars->i]);

    if (pthread_mutex_unlock(vars->sheduler->compile_done_times_mtx[vars->i]) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

static void set_dungles_free(uint64_t *next,
    sheduler_t *sheduler, uint32_t i)
{
    uint64_t    now;

    if (sheduler->dungles[left(i, sheduler->nb_coders)] == i)
    {
        run("sheduler", get_time, (void **)&(get_tm_t){&now});
        run("sheduler err", error_free, (void **)&(error_free_t){sheduler, i,
            left(i, sheduler->nb_coders), now, next});
    }
    if (sheduler->dungles[i] == i)
    {
        run("sheduler", get_time, (void **)&(get_tm_t){&now});
        run("sheduler err", error_free, (void **)&(error_free_t){sheduler, i,
            i, now, next});
    }
}

error_t set_dungles(void **args)
{
    uint32_t    i;
    sheduler_t  *sheduler;
    uint64_t    *next;

    sheduler = (sheduler_t *)args[0];
    next = (uint64_t *)args[1];
    i = 0;
    *next = sheduler->dungle_time + 1;
    while (i < sheduler->nb_coders)
    {
        if (pthread_mutex_lock(sheduler->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;

        if (*sheduler->can_take_dungles[i] != USING && *sheduler->can_take_dungles[i] != CAN_TAKE_DUNGLE)
            set_dungles_free(next, sheduler, i);

        if (pthread_mutex_unlock(sheduler->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;
        i++;
    }
    return SUCCESS;
}

static error_t start(void **args)
{
    sheduler_t  *sh;

    sh = (sheduler_t  *)args;
    if (pthread_mutex_lock(sh->start_mtx) != SUCCESS)
        return PTHREAD_ERROR;

    while (*sh->start == 0)
        if (pthread_cond_wait(sh->start_cond, sh->start_mtx) != SUCCESS)
            return PTHREAD_ERROR;
    
    if (pthread_mutex_unlock(sh->start_mtx) != SUCCESS)
        return PTHREAD_ERROR;

    if (pthread_mutex_lock(sh->start_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_cond_wait(sh->start_cond, sh->start_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_mutex_unlock(sh->start_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

static error_t wait_next(void **args)
{
    sheduler_t *sheduler;
    uint64_t   next;
    int        err;
    struct timespec ts;

    sheduler = (sheduler_t *)args;
    run("sheduler", set_dungles, (void*[]){sheduler, &next});

    if (pthread_mutex_lock(&sheduler->wake_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (next == sheduler->dungle_time + 1)
    {
        if (pthread_cond_wait(&sheduler->wake_cond, &sheduler->wake_mtx) != SUCCESS)
            return PTHREAD_ERROR;
    }
    else
    {
        ms_to_abstime(&ts, next); // use run
        err = pthread_cond_timedwait(&sheduler->wake_cond, &sheduler->wake_mtx, &ts);
        if (err != SUCCESS && err != ETIMEDOUT)
           return PTHREAD_ERROR;
    }
    if (pthread_mutex_unlock(&sheduler->wake_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

void *sheduler_runtime(void *arg) {
    sheduler_t *sheduler;
    uint32_t   could_compile;
    uint64_t    next;

    sheduler = (sheduler_t *)arg;
    run("sheduler", start, (void **)sheduler);    
    while (1) {
        run("sheduler", set_dungles, (void*[]){sheduler, &next});

        could_compile = 1;

        while (could_compile)
            run("sheduler ass", assigne_one, (void **)&(assign_t){&could_compile, sheduler});

        run("sheduler wait", wait_next, (void **)sheduler);
    }
}

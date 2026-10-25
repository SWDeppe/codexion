#include "codexion.h"
#include "parser.h"
#include "heapq.h"
#include "realloc.h"
#include "errors.h"


typedef struct get_time_s {
    uint64_t        *ms_time;
} get_tm_t;

static error_t get_time(void **args)
{
    get_tm_t    *vars;
    struct timeval tv;

    vars = (get_tm_t *)args;
    if (gettimeofday(&tv, NULL) != 0)
        return GET_TIME_OF_DAY_ERROR;
    *vars->ms_time = tv.tv_usec / 1000 + tv.tv_sec * 1000;
    return SUCCESS;
}


static void ms_to_abstime(struct timespec *ts, uint64_t add_ms)
{
    struct timeval  tv;
    uint64_t        us;

    gettimeofday(&tv, NULL);
    us = (uint64_t)tv.tv_usec + (add_ms % 1000) * 1000;
    ts->tv_sec = tv.tv_sec + add_ms / 1000 + us / 1000000;
    ts->tv_nsec = (us % 1000000) * 1000;
}

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
    // sheduler_t  *sh;
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
            *vars->could_compile = 1;
            pthread_mutex_lock(vars->sh->can_take_mtx[left(i, vars->sh->nb_coders)]);
            count += *vars->sh->can_take_dungles[left(i, vars->sh->nb_coders)] == WAITING;
            pthread_mutex_unlock(vars->sh->can_take_mtx[left(i, vars->sh->nb_coders)]);
            
            pthread_mutex_lock(vars->sh->can_take_mtx[i]);
            count += (*vars->sh->can_take_dungles[i] == WAITING) * 2;
            pthread_mutex_unlock(vars->sh->can_take_mtx[i]);
            
            pthread_mutex_lock(vars->sh->can_take_mtx[right(i, vars->sh->nb_coders)]);
            count += (*vars->sh->can_take_dungles[right(i, vars->sh->nb_coders)] == WAITING) * 4;
            pthread_mutex_unlock(vars->sh->can_take_mtx[right(i, vars->sh->nb_coders)]);
            // print("not can take", NULL, 0);

            // print("%i l", (uint32_t []){(uint32_t)(((count >> 1) & 1))}, 1);
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
            // print("%i kk", &count, 1);
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

uint64_t set_dungles_stat(sheduler_t *sheduler)
{
    uint32_t    i;
    uint64_t    now;
    uint64_t    next;

    i = 0;
    next = -1;
    while (i < sheduler->nb_coders)
    {
        pthread_mutex_lock(sheduler->can_take_mtx[i]);
        if (*sheduler->can_take_dungles[i] != USING && *sheduler->can_take_dungles[i] != CAN_TAKE_DUNGLE)
        {
            if (i == 0)
            {
                if (sheduler->dungles[sheduler->nb_coders - 1] == i)
                {
                    run("sheduler", get_time, (void **)&(get_tm_t){&now});
                    pthread_mutex_lock(sheduler->compile_done_times_mtx[i]);
                    
                    if (now - *sheduler->compile_done_times[i] >= sheduler->dungle_time)
                        sheduler->dungles[sheduler->nb_coders - 1] = DUNGLE_UNUSED;
                    else if (next > sheduler->dungle_time - now - *sheduler->compile_done_times[i])
                        next = sheduler->dungle_time - (now - *sheduler->compile_done_times[i]);
                    pthread_mutex_unlock(sheduler->compile_done_times_mtx[i]);
                }
            }
            else
                if (sheduler->dungles[i - 1] == i)
                {
                    run("sheduler", get_time, (void **)&(get_tm_t){&now});
                    pthread_mutex_lock(sheduler->compile_done_times_mtx[i]);
                    
                    if (now - *sheduler->compile_done_times[i] >= sheduler->dungle_time)
                        sheduler->dungles[i - 1] = DUNGLE_UNUSED;
                    else if (next > sheduler->dungle_time - now - *sheduler->compile_done_times[i])
                        next = sheduler->dungle_time - (now - *sheduler->compile_done_times[i]);
                        
                    pthread_mutex_unlock(sheduler->compile_done_times_mtx[i]);
                }

            if (sheduler->dungles[i] == i)
            {
                run("sheduler", get_time, (void **)&(get_tm_t){&now});
                pthread_mutex_lock(sheduler->compile_done_times_mtx[i]);
                
                if (now - *sheduler->compile_done_times[i] >= sheduler->dungle_time)
                    sheduler->dungles[i] = DUNGLE_UNUSED;
                else if (next > sheduler->dungle_time - now - *sheduler->compile_done_times[i])
                        next = sheduler->dungle_time - (now - *sheduler->compile_done_times[i]);
                    
                pthread_mutex_unlock(sheduler->compile_done_times_mtx[i]);
            }
        }
        pthread_mutex_unlock(sheduler->can_take_mtx[i]);
        i++;
    }
    if (next == sheduler->dungle_time + 1)
        return 0;
    return next;
}

void *fifo_sheduler(void *arg) {
    sheduler_t *sheduler;
    uint64_t   next;
    uint32_t   could_compile;
    struct timespec ts;

    sheduler = (sheduler_t *)arg;
    
    pthread_mutex_lock(sheduler->start_mtx);
    while (*sheduler->start == 0)
        pthread_cond_wait(sheduler->start_cond, sheduler->start_mtx);
    pthread_mutex_unlock(sheduler->start_mtx);

    pthread_mutex_lock(sheduler->start_mtx);
    pthread_cond_wait(sheduler->start_cond, sheduler->start_mtx);
    pthread_mutex_unlock(sheduler->start_mtx);
    
    while (1) {
        // v2
        next = set_dungles_stat(sheduler);
        could_compile = 1;

        while (could_compile)
            run("sheduler", assigne_one, (void **)&(assign_t){&could_compile, sheduler});

        next = set_dungles_stat(sheduler);
        pthread_mutex_lock(&sheduler->wake_mtx);
        if (next == (uint64_t)-1)
            pthread_cond_wait(&sheduler->wake_cond, &sheduler->wake_mtx);
        else
        {
            ms_to_abstime(&ts, next);
            pthread_cond_timedwait(&sheduler->wake_cond, &sheduler->wake_mtx, &ts);
        }
        pthread_mutex_unlock(&sheduler->wake_mtx);
    }
}

void *fifo_coder(void *arg) {
    coder_t     *coder;
    uint64_t    ms_time;
    // struct timeval tv;

    coder = (coder_t *)arg;
    // print("%i ,", (uint32_t)coder->start, 1);

    pthread_mutex_lock(coder->start_mtx);
    while (*coder->start == 0)
        pthread_cond_wait(coder->start_cond, coder->start_mtx);

    coder->compile_done_time = *coder->start;
    pthread_mutex_unlock(coder->start_mtx);
        // usleep(100);
    
    pthread_mutex_lock(coder->start_mtx);

    pthread_mutex_lock(&coder->can_take_mtx);
    coder->can_take_dungles = NOT_ASKING;
    pthread_mutex_unlock(&coder->can_take_mtx);

    // pthread_mutex_lock(coder->start_mtx);

    pthread_cond_wait(coder->start_cond, coder->start_mtx);
    pthread_mutex_unlock(coder->start_mtx);

    
    // ask for dungle 
    while (1)
    {
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        
        // set the ask time
        pthread_mutex_lock(&coder->ask_time_mtx);
        coder->ask_time = ms_time;
        pthread_mutex_unlock(&coder->ask_time_mtx);

        // say that its asking for dungles
        if (coder->compile_count >= coder->demanded_compilations)
        {
            pthread_mutex_lock(&coder->can_take_mtx);
            coder->can_take_dungles = ENOUTH_COMPILATIONS;
            pthread_mutex_unlock(&coder->can_take_mtx);
        }
        else
        {
            pthread_mutex_lock(&coder->can_take_mtx);
            
            coder->can_take_dungles = WAITING;
            pthread_mutex_unlock(&coder->can_take_mtx);
            
            pthread_mutex_lock(coder->wake_shedu_mtx);
            pthread_cond_broadcast(coder->wake_shedu_cond);
            pthread_mutex_unlock(coder->wake_shedu_mtx);
        }
 

        pthread_mutex_lock(coder->error_stop_mtx);
        if (*coder->error_code != SUCCESS)
        {
            // print("error occured", NULL, 0);
            pthread_mutex_unlock(coder->error_stop_mtx);
            pthread_mutex_unlock(&coder->can_take_mtx);
            return NULL;
        }
        pthread_mutex_unlock(coder->error_stop_mtx);
        
        // wait that access is given
        
        pthread_mutex_lock(&coder->can_take_mtx);
        while (coder->can_take_dungles != CAN_TAKE_DUNGLE &&
               coder->can_take_dungles != SIM_STOP)
            pthread_cond_wait(&coder->can_take_cond, &coder->can_take_mtx);
        // pthread_mutex_lock(&coder->can_take_mtx);
        
        pthread_mutex_lock(coder->error_stop_mtx);
        if (*coder->error_code != SUCCESS)
        {
            // print("error occured", NULL, 0);
            pthread_mutex_unlock(coder->error_stop_mtx);
            pthread_mutex_unlock(&coder->can_take_mtx);
            return NULL;
        }
        pthread_mutex_unlock(coder->error_stop_mtx);
        
        // change stat
        // pthread_mutex_lock(&coder->can_take_mtx);
        coder->used_dungle[0] = coder->dungles[0];
        coder->used_dungle[1] = coder->dungles[1];
        coder->can_take_dungles = USING;
        pthread_mutex_unlock(&coder->can_take_mtx);
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});

        print("%li coder %u got access to dungles !\n",  (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
        
        // start compiling
        print("%li coder %u is Compiling\n",  (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
        usleep(coder->waiting_times[COMPILING] * 1000);
        
        coder->compile_count++;
        

        // set the new time of not compiling
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        
        pthread_mutex_lock(&coder->compile_done_time_mtx);
        coder->compile_done_time = ms_time;
        pthread_mutex_unlock(&coder->compile_done_time_mtx);
        pthread_mutex_lock(coder->wake_monit_mtx);
        pthread_cond_broadcast(coder->wake_monit_cond);
        pthread_mutex_unlock(coder->wake_monit_mtx);

        pthread_mutex_lock(coder->wake_shedu_mtx);
        pthread_cond_broadcast(coder->wake_shedu_cond);
        pthread_mutex_unlock(coder->wake_shedu_mtx);

        pthread_mutex_lock(&coder->can_take_mtx);
        coder->can_take_dungles = NOT_ASKING;
        pthread_mutex_unlock(&coder->can_take_mtx);
        
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});

        print("%lu coder %u is refacotring\n", (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
        coder->used_dungle[0] = DUNGLE_UNUSED;
        coder->used_dungle[1] = DUNGLE_UNUSED;
        usleep(coder->waiting_times[REFACTORING] * 1000);
    
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        print("%lu coder %u is Debuging\n", (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
        usleep(coder->waiting_times[DEBUGING] * 1000);
    }
}

void cleanup(codexion_t *codexion)
{
    uint32_t    i;

    i = 0;
    while (i < codexion->nb_coders)
    {
        pthread_mutex_destroy(codexion->sheduler.ask_times_mtx[i]);
        pthread_mutex_destroy(codexion->sheduler.can_take_mtx[i]);
        pthread_mutex_destroy(codexion->sheduler.compile_done_times_mtx[i]);
        pthread_cond_destroy(codexion->sheduler.can_take_cond[i]);
    }
    free(codexion->sheduler.ask_times);
    free(codexion->sheduler.ask_times_mtx);
    free(codexion->sheduler.can_take_cond);
    free(codexion->sheduler.can_take_dungles);
    free(codexion->sheduler.can_take_mtx);
    free(codexion->sheduler.compile_done_times);
    free(codexion->sheduler.compile_done_times_mtx);
    free(codexion->sheduler.dungles);
    free(codexion->sheduler.dungles_priority);
    free(codexion->sheduler.compile_done_times);
    free(codexion->coders);
}



void *monitor_runtime(void *arg)
{
    monitor_t   *monitor;
    uint32_t    coder_done_count;
    uint32_t    i;
    uint64_t    ms_time;
    struct timespec    ts;

    monitor = (monitor_t *)arg;
    pthread_mutex_lock(&monitor->start_mtx);
    run("Monitor", get_time, (void **)&(get_tm_t){&monitor->start});
    pthread_mutex_unlock(&monitor->start_mtx);
    pthread_cond_broadcast(&monitor->start_cond);

    // pthread_cond_broadcast(monitor->wake_shedu_cond);

    // usleep(100);
    coder_done_count = 1;
    while (coder_done_count)
    {
        i = 0;
        coder_done_count = monitor->nb_coders;
        while (i < monitor->nb_coders)
        {
            pthread_mutex_lock(monitor->can_take_mtx[i]);
            if (*monitor->can_take_dungles[i] == NOT_ASKING)
                coder_done_count--;
            pthread_mutex_unlock(monitor->can_take_mtx[i]);
            i++;
        }
    }
    pthread_mutex_lock(&monitor->start_mtx);
    pthread_mutex_unlock(&monitor->start_mtx);
    pthread_cond_broadcast(&monitor->start_cond);
    while (1)
    {
        // v2
        monitor->min_t = monitor->burount_time;
        i = 0;
        coder_done_count = 0;
        while (i < monitor->nb_coders)
        {
            pthread_mutex_lock(monitor->can_take_mtx[i]);
            if (*monitor->can_take_dungles[i] == ENOUTH_COMPILATIONS)
            {
                coder_done_count++;
                continue;
            }
            pthread_mutex_unlock(monitor->can_take_mtx[i]);
            run("Monitor", get_time, (void **)&(get_tm_t){&ms_time});
            pthread_mutex_lock(monitor->compile_done_times_mtx[i]);
            if (*monitor->compile_done_times[i] != 0 && ms_time - *monitor->compile_done_times[i] >= monitor->burount_time)
            {
                monitor->error_code = CODER_DIED;
                print("coder %i died at %i !", (uint32_t []){i, (uint32_t)ms_time - monitor->start}, 2);
            }
            else
            {
                monitor->dies_in[i] = ms_time - *monitor->compile_done_times[i];
                if (monitor->dies_in[i] < monitor->min_t)
                    monitor->min_t = monitor->dies_in[i];
            }
            pthread_mutex_unlock(monitor->compile_done_times_mtx[i]);
            i++;
        }
        // stop if everyone finished
        if (coder_done_count == monitor->nb_coders)
            monitor->error_code = SIM_STOP;

        // stop simulation if error is detected
        pthread_mutex_lock(&monitor->error_stop_mtx);
        if (monitor->error_code != SUCCESS)
        {
            i = 0;
            while (i < monitor->nb_coders)
            {
                pthread_mutex_lock(monitor->can_take_mtx[i]);
                *monitor->can_take_dungles[i] = (uint8_t)SIM_STOP;
                pthread_cond_broadcast(monitor->can_take_cond[i]);
                pthread_mutex_unlock(monitor->can_take_mtx[i]);
                i++;
            }
            pthread_mutex_unlock(&monitor->error_stop_mtx);
            return NULL;
        }
        pthread_mutex_unlock(&monitor->error_stop_mtx);
        
        // wait time of the first one to die
        ms_to_abstime(&ts, monitor->min_t); // todo use it error safe
        pthread_mutex_lock(&monitor->wake_mtx);
        pthread_cond_timedwait(&monitor->wake_cond, &monitor->wake_mtx, &ts);
        pthread_mutex_unlock(&monitor->wake_mtx);
    }
}


error_t gen_coder(coder_t *coder, pars_t parsed, uint32_t id)
{
    // TODO: Error management
    // ask time
    if (gettimeofday(&coder->tv, NULL) != 0)
        return GET_TIME_OF_DAY_ERROR;
    coder->ask_time = coder->tv.tv_sec * 1000 + coder->tv.tv_usec / 1000;
    pthread_mutex_init(&(coder->ask_time_mtx), NULL);
    // can take dungle
    coder->can_take_dungles = STARTUP;
    pthread_mutex_init(&coder->can_take_mtx, NULL);
    pthread_cond_init(&coder->can_take_cond, NULL);
    pthread_mutex_init(&coder->compile_done_time_mtx, NULL);
    // coder->start = (uint64_t)tv->tv_sec * 1000 + tv->tv_usec / 1000;
    coder->compile_done_time = 0;
    coder->compile_count = 0;
    coder->demanded_compilations = parsed.required_compil;

    if (id == 0)
    coder->dungles[0] = parsed.nb_coders - 1;
    else
    coder->dungles[0] = id - 1;
    coder->dungles[1] = id;
    
    coder->used_dungle[0] = DUNGLE_UNUSED;
    coder->used_dungle[1] = DUNGLE_UNUSED;
    coder->id = id;
    coder->waiting_times[COMPILING] = parsed.compile_time;
    coder->waiting_times[DEBUGING] = parsed.debug_time;
    coder->waiting_times[REFACTORING] = parsed.refactor_time;
    // pthread_create(&coder->coder_thread, NULL, fifo_coder, coder);
    return SUCCESS;
}

error_t gen_base_data(codexion_t *codexion, pars_t parsed)
{
    uint32_t    i;
    struct timeval  tv;

    // allocate memory
    if (gettimeofday(&tv, NULL) != 0)
        return GET_TIME_OF_DAY_ERROR;
    // codexion = (codexion_t *)malloc(sizeof(codexion_t));
    codexion->nb_coders = parsed.nb_coders;
    codexion->coders = (coder_t *)malloc(sizeof(coder_t) * parsed.nb_coders);
    print("__init__", NULL, 0);
    // generate coders
    i = 0;
    pthread_mutex_init(&codexion->sheduler.wake_mtx, NULL); // check errors
    pthread_cond_init(&codexion->sheduler.wake_cond, NULL); // check errors

    pthread_mutex_init(&codexion->monitor.wake_mtx, NULL); // check errors
    pthread_cond_init(&codexion->monitor.wake_cond, NULL); // check errors

    pthread_mutex_init(&codexion->monitor.start_mtx, NULL); // check errors
    pthread_cond_init(&codexion->monitor.start_cond, NULL); // check errors

    while (i < parsed.nb_coders)
    {
        gen_coder(&codexion->coders[i], parsed, i); // TODO: add proper error handeling
        codexion->coders[i].wake_shedu_cond = &codexion->sheduler.wake_cond;
        codexion->coders[i].wake_shedu_mtx = &codexion->sheduler.wake_mtx;
        codexion->coders[i].wake_monit_cond = &codexion->monitor.wake_cond;
        codexion->coders[i].wake_monit_mtx = &codexion->monitor.wake_mtx;
        codexion->coders[i].start_cond = &codexion->monitor.start_cond;
        codexion->coders[i].start_mtx = &codexion->monitor.start_mtx;
        codexion->coders[i].start = &codexion->monitor.start;
        i++;
    }
    // make sheduler threads
    codexion->sheduler.ask_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->sheduler.ask_times = (uint64_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->sheduler.dungle_time = parsed.dungle_time;
    codexion->sheduler.start = &codexion->monitor.start;
    codexion->sheduler.start_cond = &codexion->monitor.start_cond;
    codexion->sheduler.start_mtx = &codexion->monitor.start_mtx;
    
    codexion->sheduler.can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * parsed.nb_coders);
    
    codexion->monitor.can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * parsed.nb_coders);
    
    codexion->sheduler.can_take_dungles = (uint8_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.can_take_dungles = (uint8_t **)malloc(sizeof(void *) * parsed.nb_coders);
    
    codexion->sheduler.can_take_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.can_take_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);

    codexion->sheduler.compile_done_times = (uint64_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.compile_done_times = (uint64_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->sheduler.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);


    codexion->sheduler.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->monitor.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);

    // codexion->sheduler.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    // codexion->monitor.compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);

    codexion->sheduler.dungles = (uint32_t *)malloc(sizeof(uint32_t) * parsed.nb_coders);
    codexion->sheduler.dungles_priority = malloc(sizeof *codexion->sheduler.dungles_priority * parsed.nb_coders);
    codexion->sheduler.compile_done_times = (uint64_t **)malloc(sizeof(void *) * parsed.nb_coders);

    codexion->monitor.min_t = parsed.burnout_time;codexion->monitor.dies_in = malloc(sizeof(uint64_t) * parsed.nb_coders);
    codexion->monitor.wake_shedu_cond = &codexion->monitor.wake_cond;
    codexion->monitor.wake_shedu_mtx = &codexion->monitor.wake_mtx;
    codexion->monitor.dies_in = malloc(sizeof(uint64_t) * parsed.nb_coders);

    // init monitor
    codexion->monitor.burount_time = parsed.burnout_time;
    pthread_mutex_init(&codexion->monitor.error_stop_mtx, NULL); // check errors
    codexion->sheduler.error_code = &codexion->monitor.error_code;
    codexion->sheduler.error_stop_mtx = &codexion->monitor.error_stop_mtx;
    codexion->monitor.nb_coders = parsed.nb_coders;
    codexion->monitor.error_code = SUCCESS;
    codexion->monitor.start = 0;
    i = 0;
    while (i < parsed.nb_coders)
    {
        codexion->coders[i].error_code = &codexion->monitor.error_code;
        codexion->coders[i].error_stop_mtx = &codexion->monitor.error_stop_mtx;

        codexion->sheduler.ask_times[i] = &codexion->coders[i].ask_time;
        codexion->sheduler.ask_times_mtx[i] = &codexion->coders[i].ask_time_mtx;

        codexion->sheduler.can_take_cond[i] = &codexion->coders[i].can_take_cond;
        codexion->monitor.can_take_cond[i] = &codexion->coders[i].can_take_cond;
        codexion->sheduler.can_take_dungles[i] = &codexion->coders[i].can_take_dungles;
        codexion->monitor.can_take_dungles[i] = &codexion->coders[i].can_take_dungles;
        codexion->sheduler.can_take_mtx[i] = &codexion->coders[i].can_take_mtx;
        codexion->monitor.can_take_mtx[i] = &codexion->coders[i].can_take_mtx;

        codexion->sheduler.compile_done_times[i] = &codexion->coders[i].compile_done_time;
        codexion->monitor.compile_done_times[i] = &codexion->coders[i].compile_done_time;

        codexion->sheduler.compile_done_times_mtx[i] = &codexion->coders[i].compile_done_time_mtx;
        codexion->monitor.compile_done_times_mtx[i] = &codexion->coders[i].compile_done_time_mtx;

        codexion->sheduler.dungles[i] = DUNGLE_UNUSED;
        codexion->sheduler.dungles_priority[i][0] = FREE_SPACE;
        codexion->sheduler.dungles_priority[i][1] = FREE_SPACE;
        codexion->sheduler.nb_coders = parsed.nb_coders;
        
        i++;
    }

    return SUCCESS;
}

error_t    *launch(codexion_t *codexion)
{
    uint32_t    i;
    error_t     *errors;

    i = 0;
    errors = (error_t *)malloc((codexion->nb_coders + 2) * sizeof(error_t));
    if (errors == NULL)
        return (error_t *)MALLOC_ERROR;
    errors[codexion->nb_coders + 1] = ERROR_LIST_END;
    pthread_create(&codexion->sheduler.shedulter_thread, NULL, fifo_sheduler, &codexion->sheduler);
    pthread_create(&codexion->monitor.thread, NULL, monitor_runtime, &codexion->monitor);
    while (i < codexion->nb_coders)
    {
        pthread_create(&codexion->coders[i].coder_thread, NULL, fifo_coder, &codexion->coders[i]);
        i++;
    }
    pthread_join(codexion->monitor.thread, NULL);
    pthread_join(codexion->sheduler.shedulter_thread, NULL);
    i = 0;
    while (i < codexion->nb_coders)
    {
        pthread_join(codexion->coders[i].coder_thread, NULL);
        i++;
    }
    return errors;
}



void log_errors(error_t *errors, uint32_t *i)
{
    *i = 0;
    while (errors[*i] == SUCCESS
           && errors[*i] != ERROR_LIST_END)
        (*i)++;
    return;
}

int main(int argc, char **argv) {
    pars_t      parsing;
    // error_t     *errors;
    codexion_t  proj_holder;
    // uint32_t    i;

    // TODO: manage the way the errors are handeled
    parse(argc, argv, &parsing);
    gen_base_data(&proj_holder, parsing);
    launch(&proj_holder);
}

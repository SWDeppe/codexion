#include "codexion.h"
#include "parser.h"
#include "heapq.h"
#include "errors.h"

typedef struct make_priorities_s{
    uint32_t *waiters;
    uint32_t nb_waiters;
    sheduler_t *sheduler;
} mk_prios_t;

// FIFO
error_t make_priorities(uint32_t *waiters, uint32_t nb_waiters, sheduler_t *sheduler) {
    uint32_t    i;
    uint32_t    current;
    heapq_t     prior_q;
    mk_prios_t  *vars;

    // vars = (mk_prios_t *)args;
    run("sheduler", init_heapq, (void **)&(i_heapq_t){&prior_q, 0, NULL, NULL, vars->nb_waiters});
    i = 0;
    while (i < vars->nb_waiters)
    {
        pthread_mutex_lock((vars->sheduler->ask_times_mtx)[vars->waiters[i]]);
        heapq_insert(&prior_q, (*vars->sheduler->ask_times)[vars->waiters[i]], vars->waiters[i]);
        pthread_mutex_unlock((vars->sheduler->ask_times_mtx)[vars->waiters[i]]);
        if (vars->waiters[i] == 0) {
            vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][0] = FREE_SPACE;
            vars->sheduler->dungles_priority[0][0] = FREE_SPACE;
        }
        else {
            vars->sheduler->dungles_priority[vars->waiters[i]][0] = FREE_SPACE;
            vars->sheduler->dungles_priority[
                vars->waiters[i] - 1
            ][0] = FREE_SPACE;
        }
        i++;
    }
    i = 0;
    while (i < vars->nb_waiters)

    {
        current = pop(&prior_q);
        // // // // printf("at i %u got coder %u\n", i, current);
        if (current == 0)
        {
            if (vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][0] == FREE_SPACE &&
                vars->sheduler->dungles_priority[0][0] == FREE_SPACE)
            {
                vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][0] = current;
                vars->sheduler->dungles_priority[0][0] = current;
            } else
            {
                vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][1] = current;
                vars->sheduler->dungles_priority[0][1] = current;
            }
        }
        else
        {

            if (vars->sheduler->dungles_priority[current][0] == FREE_SPACE &&
                vars->sheduler->dungles_priority[current - 1][0] == FREE_SPACE)
            {
                vars->sheduler->dungles_priority[current][0] = current;
                vars->sheduler->dungles_priority[current - 1][0] = current;
            } else
            {
                vars->sheduler->dungles_priority[current][1] = current;
                vars->sheduler->dungles_priority[current - 1][1] = current;
            }
        }
        i++;
    }
    heapq_destroy(&prior_q);
    return SUCCESS;
}
// uint32_t **waiters, uint32_t *count, sheduler_t *sheduler
typedef struct get_waiter_s {
    uint32_t **waiters;
    uint32_t *count;
    sheduler_t *sheduler;
} g_waiter_t;

error_t get_waiters(void **args) {
    uint32_t    i;
    uint32_t    y;
    g_waiter_t  *var;

    var = (g_waiter_t *)args;
    if (*(var->waiters) != NULL)
        free(*var->waiters);
    *var->count = 0;
    i = 0;
    while (i < var->sheduler->nb_coders) {
        pthread_mutex_lock(var->sheduler->can_take_mtx[i]);
        if (*var->sheduler->can_take_dungles[i] == WAITING)
            (*var->count)++;
        pthread_mutex_unlock(var->sheduler->can_take_mtx[i]);
        i++;
    }
    *var->waiters = (uint32_t *)malloc(sizeof(uint32_t) * *var->count);
    if (*var->waiters == NULL)
        return MALLOC_ERROR;
    i = 0;
    y = 0;
    while (i < var->sheduler->nb_coders) {
        pthread_mutex_lock(var->sheduler->can_take_mtx[i]);
        if (*var->sheduler->can_take_dungles[i] == WAITING)
            (*var->waiters)[y++] = i;
        pthread_mutex_unlock(var->sheduler->can_take_mtx[i]);
        i++;
    }
    // // // // printf("is done with waiters");
    return SUCCESS;
}

typedef struct assigne_dungles_s {
    sheduler_t *sheduler;
     uint32_t **new_dungles_poss;
      uint32_t *poss_count;
} asgn_dung_t;

error_t assigne_dungles(void **args) {
    // TODO: Add proper error management
    uint32_t    i;
    uint32_t    y;
    asgn_dung_t *vars;

    vars = (asgn_dung_t *)args;
    i = 0;
    *vars->poss_count = 0;
    while (i < vars->sheduler->nb_coders)
    {
        if (i == 0)
        {
            if (vars->sheduler->dungles[vars->sheduler->nb_coders - 1] == DUNGLE_UNUSED &&
                vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][0] == i && 
                vars->sheduler->dungles[i] == DUNGLE_UNUSED &&
                vars->sheduler->dungles_priority[i][0] == i)
                (*vars->poss_count)++;

        }
        else if (vars->sheduler->dungles[i - 1] == DUNGLE_UNUSED &&
                 vars->sheduler->dungles_priority[i - 1][0] == i && 
                 vars->sheduler->dungles[i] == DUNGLE_UNUSED &&
                 vars->sheduler->dungles_priority[i][0] == i)
            (*vars->poss_count)++;
        i++;
    }
    if (*vars->new_dungles_poss)
        free(*vars->new_dungles_poss);
    *vars->new_dungles_poss = (uint32_t *)malloc(sizeof(uint32_t) * *vars->poss_count);
    if (*vars->new_dungles_poss == NULL)
        return MALLOC_ERROR;
    i = 0;
    y = 0;
    while (i < vars->sheduler->nb_coders)
    {
        if (i == 0)
        {
            if (vars->sheduler->dungles[vars->sheduler->nb_coders - 1] == DUNGLE_UNUSED &&
                vars->sheduler->dungles_priority[vars->sheduler->nb_coders - 1][0] == i && 
                vars->sheduler->dungles[i] == DUNGLE_UNUSED &&
                vars->sheduler->dungles_priority[i][0] == i)
                (*vars->new_dungles_poss)[y++] = i;

        }
        else if (vars->sheduler->dungles[i - 1] == DUNGLE_UNUSED &&
                 vars->sheduler->dungles_priority[i - 1][0] == i && 
                 vars->sheduler->dungles[i] == DUNGLE_UNUSED &&
                 vars->sheduler->dungles_priority[i][0] == i)
            (*vars->new_dungles_poss)[y++] = i;
        i++;
    }
    y = 0;
    return SUCCESS;
}

void set_dungles_stat(sheduler_t *sheduler)
{
    uint32_t    i;

    i = 0;
    while (i < sheduler->nb_coders)
    {
        pthread_mutex_lock(sheduler->can_take_mtx[i]);
        if (*sheduler->can_take_dungles[i] == NOT_ASKING)
        {
            if (i == 0)
                sheduler->dungles[sheduler->nb_coders - 1] = DUNGLE_UNUSED;
            else
                sheduler->dungles[i - 1] = DUNGLE_UNUSED;
            sheduler->dungles[i] = DUNGLE_UNUSED;
        }
        pthread_mutex_unlock(sheduler->can_take_mtx[i]);
        i++;
    }
}

void *fifo_sheduler(void *arg) {
    sheduler_t *sheduler;
    uint32_t   *waiters;
    uint32_t   *new_poss;
    uint32_t   count;

    sheduler = (sheduler_t *)arg;
    new_poss = NULL;
    waiters = NULL;
    while (1) {

        pthread_mutex_lock(sheduler->error_stop_mtx);
        if (*sheduler->error_code != SUCCESS)
            return NULL;
        pthread_mutex_unlock(sheduler->error_stop_mtx);

        // look for who is waiting
        run("sheduler", &get_waiters, (void **)&(g_waiter_t){ &waiters, &count, sheduler });
        set_dungles_stat(sheduler);
        if (count == 0)
            continue;
        // check at there last compilation time
        // make a priority for each dungle
        run("sheduler", make_priorities, (void **)&(mk_prios_t){waiters, count, sheduler});  // TODO: say to monitor error happend
        
        // assigne a pair of dungles to each coder
        run("sheduler", assigne_dungles, (void **)&(asgn_dung_t){sheduler, &new_poss, &count});
        if (count == 0)
            continue;

        // broadcast the coders that got there dungles
        while (count--) {
            // // // // printf("possesor at coount is %u\n", new_poss[count]);
            if (new_poss[count] == 0)
            {
                sheduler->dungles[sheduler->nb_coders - 1] = 0;
                sheduler->dungles[0] = 0;
            }
            else
            {
                sheduler->dungles[new_poss[count] - 1] = new_poss[count];
                sheduler->dungles[new_poss[count]] = new_poss[count];
            }
            pthread_mutex_lock(sheduler->can_take_mtx[new_poss[count]]);
            *sheduler->can_take_dungles[new_poss[count]] = CAN_TAKE_DUNGLE;
            pthread_mutex_unlock(sheduler->can_take_mtx[new_poss[count]]);
            pthread_cond_broadcast(sheduler->can_take_cond[new_poss[count]]);

        }
    }
}

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

void *fifo_coder(void *arg) {
    coder_t     *coder;
    uint64_t    ms_time;

    coder = (coder_t *)arg;
   // ask for dungle 
    while (1)
    {

        // say that its asking for dungles
        pthread_mutex_lock(&coder->can_take_mtx);
        if (coder->compile_count >= coder->demanded_compilations)
            coder->can_take_dungles = ENOUTH_COMPILATIONS;
        else
            coder->can_take_dungles = WAITING;
        pthread_mutex_unlock(&coder->can_take_mtx);
        
        // if (gettimeofday(&coder->tv, NULL) != 0)
        // {
        //     pthread_mutex_lock(coder->error_stop_mtx);
        //     coder->error_code = (uint8_t *)GET_TIME_OF_DAY_ERROR;
        //     pthread_mutex_unlock(coder->error_stop_mtx);

        // }
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        // set the ask time
        pthread_mutex_lock(&coder->ask_time_mtx);
        coder->ask_time = ms_time;
        pthread_mutex_unlock(&coder->ask_time_mtx);

        
        // wait that access is given
        pthread_mutex_lock(&coder->can_take_mtx);
        while (coder->can_take_dungles != CAN_TAKE_DUNGLE &&
               coder->can_take_dungles != SIM_STOP)
            pthread_cond_wait(&coder->can_take_cond, &coder->can_take_mtx);

        
        pthread_mutex_lock(coder->error_stop_mtx);
        if (*coder->error_code != SUCCESS)
            return NULL;    
        pthread_mutex_unlock(coder->error_stop_mtx);
        // // // // printf("coder %u got threw cond !\n", coder->id);
        
        // change stat
        coder->used_dungle[0] = coder->dungles[0];
        coder->used_dungle[1] = coder->dungles[1];
        coder->can_take_dungles = USING;
        pthread_mutex_unlock(&coder->can_take_mtx);

        printf("coder %u got access to dungles !\n", coder->id);
        // printf("coder %u got access to dungles !", (uint32_t *)&coder->id);
        
        // start compiling
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});

        printf("%li coder %u is Compiling\n", ms_time - coder->start, coder->id);
        usleep(coder->waiting_times[COMPILING] * 1000);
    
        pthread_mutex_lock(&coder->can_take_mtx);
        coder->can_take_dungles = NOT_ASKING;
        pthread_mutex_unlock(&coder->can_take_mtx);

        // set the new time of not compiling
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});

        pthread_mutex_lock(&coder->compile_done_time_mtx);
        coder->compile_done_time = ms_time;
        pthread_mutex_unlock(&coder->compile_done_time_mtx);
        
        printf("%lu coder %u is refacotring\n", ms_time - coder->start, coder->id);
        coder->used_dungle[0] = DUNGLE_UNUSED;
        coder->used_dungle[1] = DUNGLE_UNUSED;
        usleep(coder->waiting_times[REFACTORING] * 1000);
    
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        printf("%lu coder %u is Debuging\n", ms_time - coder->start, coder->id);
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
    uint64_t    time;

    monitor = (monitor_t *)arg;
    while (1)
    {
        i = 0;
        coder_done_count = 0;
        while (i < monitor->nb_coders)
        {
            run("Monitor", get_time, (void **)&(get_tm_t){&time});
            pthread_mutex_lock(monitor->compile_done_times_mtx[i]);
            if (time - *monitor->compile_done_times[i] >= monitor->burount_time)
                monitor->error_code = CODER_DIED;
            pthread_mutex_unlock(monitor->compile_done_times_mtx[i]);
            pthread_mutex_lock(monitor->can_take_mtx[i]);
            if (*monitor->can_take_dungles[i] == ENOUTH_COMPILATIONS)
                coder_done_count++;
            pthread_mutex_unlock(monitor->can_take_mtx[i]);
            i++;
        }
        
        pthread_mutex_lock(&monitor->error_stop_mtx);
        if (coder_done_count == monitor->nb_coders)
        {
            monitor->error_code = SIM_STOP;
        }
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
    }
}


error_t gen_coder(coder_t *coder, pars_t parsed, struct timeval *tv, uint32_t id)
{
    // TODO: Error management
    // ask time
    if (gettimeofday(&coder->tv, NULL) != 0)
        return GET_TIME_OF_DAY_ERROR;
    coder->ask_time = tv->tv_sec * 1000 + tv->tv_usec / 1000;
    pthread_mutex_init(&(coder->ask_time_mtx), NULL);
    // can take dungle
    coder->can_take_dungles = WAITING;
    pthread_mutex_init(&coder->can_take_mtx, NULL);
    pthread_cond_init(&coder->can_take_cond, NULL);
    pthread_mutex_init(&coder->compile_done_time_mtx, NULL);
    coder->start = (uint64_t)tv->tv_sec * 1000 + tv->tv_usec / 1000;
    coder->compile_done_time = coder->start;
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
    print("__init__", NULL);
    #ifdef DEBUG
    assert(parsed.nb_coders != 0);
    assert(ret->coders != NULL);
    #endif
    // generate coders
    i = 0;
    while (i < parsed.nb_coders)
    {
        gen_coder(&codexion->coders[i], parsed, &tv, i); // TODO: add proper error handeling
        i++;
    }
    // make sheduler threads
    i = 0;
    codexion->sheduler.ask_times = (uint64_t **)malloc(sizeof(void *) * parsed.nb_coders);
    codexion->sheduler.ask_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * parsed.nb_coders);
    
    codexion->sheduler.can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * parsed.nb_coders);
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

    // init monitor
    codexion->monitor.burount_time = parsed.burnout_time;
    pthread_mutex_init(&codexion->monitor.error_stop_mtx, NULL); // check errors
    codexion->sheduler.error_code = &codexion->monitor.error_code;
    codexion->sheduler.error_stop_mtx = &codexion->monitor.error_stop_mtx;
    codexion->monitor.nb_coders = parsed.nb_coders;
    codexion->monitor.error_code = 0;
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
        get_time((void **)&(get_tm_t){&codexion->sheduler.start});
        i++;
    }
    // pthread_create(&codexion->sheduler.shedulter_thread, NULL, fifo_sheduler, &codexion->sheduler);
    // pthread_create(&codexion->monitor.thread, NULL, monitor_runtime, &codexion->monitor);

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
        // // // printf("%u coder thread launched\n", i);
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

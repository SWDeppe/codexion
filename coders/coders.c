#include "codexion.h"
#include "errors.h"

// TODO: fix the start (sheduler can start befor )
// static error_t start(void **args)
// {
//     coder_t *coder;

//     coder = (coder_t *)args;
//     if (pthread_mutex_lock(coder->start_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     while (*coder->start == 0)
//         if (pthread_cond_wait(coder->start_cond, coder->start_mtx) != SUCCESS)
//             return PTHREAD_ERROR;
//     coder->compile_done_time = *coder->start;
//     if (pthread_mutex_unlock(coder->start_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     if (pthread_mutex_lock(coder->start_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     coder->can_take_dungles = NOT_ASKING;
//     if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     if (pthread_cond_wait(coder->start_cond, coder->start_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     if (pthread_mutex_unlock(coder->start_mtx) != SUCCESS)
//         return PTHREAD_ERROR;
//     return SUCCESS;
// }

static error_t start(void **args)
{
    coder_t *coder;

    coder = (coder_t *)args;
    pthread_mutex_lock(&coder->can_take_mtx);
    coder->can_take_dungles = NOT_ASKING;
    pthread_mutex_unlock(&coder->can_take_mtx);
    pthread_mutex_lock(coder->start_mtx);
    pthread_cond_wait(coder->start_cond, coder->start_mtx);
    pthread_mutex_unlock(coder->start_mtx);
    return SUCCESS;
}

static error_t wake_sheduler(void **args)
{
    coder_t *coder;

    coder = (coder_t *)args;
    if (pthread_mutex_lock(coder->wake_shedu_mtx) != SUCCESS)
                return PTHREAD_ERROR;
    if (pthread_cond_broadcast(coder->wake_shedu_cond) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_mutex_unlock(coder->wake_shedu_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

static error_t set_stat(void **args)
{
    coder_t *coder;
    uint8_t *stop;

    coder = (coder_t *)args[0];
    stop = (uint8_t *)args[1];
    if (coder->compile_count >= coder->demanded_compilations)
    {
        if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
            return PTHREAD_ERROR;
        coder->can_take_dungles = ENOUTH_COMPILATIONS;
        if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
            return PTHREAD_ERROR;
        *stop = 1;
    }
    else
    {
        if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
            return PTHREAD_ERROR;
        coder->can_take_dungles = WAITING;
        if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
            return PTHREAD_ERROR;
        run("Coder", wake_sheduler, (void **)coder);
    }
    return SUCCESS;
}

error_t error_check(void **args)
{
    coder_t *coder;
    uint8_t *stop;

    coder = (coder_t *)args[0];
    stop = (uint8_t *)args[1];
    if (pthread_mutex_lock(coder->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (*coder->error_code != SUCCESS)
        *stop = 1;
    if (pthread_mutex_unlock(coder->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

error_t wait_for_access(void **args)
{
    coder_t *coder;
    
    coder = (coder_t *)args;
    if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    while (coder->can_take_dungles != CAN_TAKE_DUNGLE &&
            coder->can_take_dungles != SIM_STOP)
        if (pthread_cond_wait(&coder->can_take_cond, &coder->can_take_mtx) != SUCCESS)
           return PTHREAD_ERROR;
    // set new state
    coder->used_dungle[0] = coder->dungles[0];
    coder->used_dungle[1] = coder->dungles[1];
    coder->can_take_dungles = USING;
    if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

static error_t set_times_to_broadcast(void **args)
{
    coder_t     *coder;
    uint64_t    *ms_time;
    
    coder = (coder_t *)args[0];
    ms_time = (uint64_t *)args[1];
    run("Coder", get_time, (void **)&(get_tm_t){ms_time});
    if (pthread_mutex_lock(&coder->compile_done_time_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    coder->compile_done_time = *ms_time;
    if (pthread_mutex_unlock(&coder->compile_done_time_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_mutex_lock(coder->wake_monit_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_cond_broadcast(coder->wake_monit_cond) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_mutex_unlock(coder->wake_monit_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    usleep(coder->waiting_times[COMPILING] * 1000);
    if (pthread_mutex_lock(coder->wake_shedu_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_cond_broadcast(coder->wake_shedu_cond) != SUCCESS)
        return PTHREAD_ERROR;
    if (pthread_mutex_unlock(coder->wake_shedu_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

static error_t free_dung_and_refactor(void **args)
{
    coder_t     *coder;
    uint64_t    *ms_time;

    coder = (coder_t *)args[0];
    ms_time = (uint64_t *)args[1];
    if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    coder->can_take_dungles = NOT_ASKING;
    if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    run("Coder", get_time, (void **)&(get_tm_t){ms_time});
    print("%lu coder %u is refacotring\n", (uint32_t []){ *ms_time - *coder->start, coder->id }, 2);
    coder->used_dungle[0] = DUNGLE_UNUSED;
    coder->used_dungle[1] = DUNGLE_UNUSED;
    usleep(coder->waiting_times[REFACTORING] * 1000);
    return SUCCESS;
}

static int use_dungles(coder_t *coder, uint64_t ms_time)
{
    uint8_t stop;

    stop = 0;
    print("%li coder %u got access to dungle %i !\n",(uint32_t []){
        ms_time - *coder->start, coder->id, coder->dungles[0] }, 3);
    print("%li coder %u got access to dungle %i !\n",  (uint32_t []){
        ms_time - *coder->start, coder->id, coder->dungles[1] }, 3);
    print("%li coder %u is Compiling\n",  (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
    coder->compile_count++;
    run("Coder", set_times_to_broadcast, (void *[]){coder, &ms_time});
    run("Coder", error_check, (void *[]){coder, &stop});
    if (stop)
        return 1;
    run("Coder", free_dung_and_refactor, (void *[]){coder, &ms_time});
    run("Coder", error_check, (void *[]){coder, &stop});
    if (stop)
        return 1;
    run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
    print("%lu coder %u is Debuging\n", (uint32_t []){ ms_time - *coder->start, coder->id }, 2);
    usleep(coder->waiting_times[DEBUGING] * 1000);
    run("Coder", error_check, (void *[]){coder, &stop});
    if (stop)
        return 1;
    return 0;
}

void *coder_runtime(void *arg)
{
    coder_t     *coder;
    uint64_t    ms_time;
    uint8_t     stop;

    coder = (coder_t *)arg;
    run("Coder", start, (void **)coder);
    stop = 0;
    while (1)
    {
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        pthread_mutex_lock(&coder->ask_time_mtx);
        coder->ask_time = ms_time;
        pthread_mutex_unlock(&coder->ask_time_mtx);
        run("Coder", set_stat, (void *[]){coder, &stop});
        run("Coder", error_check, (void *[]){coder, &stop});
        if (stop)
            return NULL;
        run("Coder", wait_for_access, (void **)coder);
        run("Coder", error_check, (void *[]){coder, &stop});
        if (stop)
            return NULL;
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        if (use_dungles(coder, ms_time))
            return NULL;
    }
}
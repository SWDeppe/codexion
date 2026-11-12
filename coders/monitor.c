#include "codexion.h"

static error_t start(void **args)
{
    monitor_t *monitor;
    uint32_t i;
    uint32_t count;

    monitor = (monitor_t *)args;
    count = 0;
    while (count < monitor->nb_coders + 1)
    {
        count = 0;
        i = 0;
        while (i < monitor->nb_coders)
        {
            pthread_mutex_lock(monitor->can_take_mtx[i]);
            if (*monitor->can_take_dungles[i] == NOT_ASKING)
                count++;
            pthread_mutex_unlock(monitor->can_take_mtx[i]);
            i++;
        }
        pthread_mutex_lock(monitor->shedu_ready_mtx);
        if (*monitor->shedu_ready == 1)
            count++;
        pthread_mutex_unlock(monitor->shedu_ready_mtx);
    }
    usleep(500);
    run("Monitor", get_time, (void **)&(get_tm_t){&monitor->start});
    i = 0;
    while (i < monitor->nb_coders)
    {
        pthread_mutex_lock(monitor->compile_done_times_mtx[i]);
        *monitor->compile_done_times[i] = monitor->start;
        pthread_mutex_unlock(monitor->compile_done_times_mtx[i]);
        i++;
    }
    // pthread_mutex_lock(&monitor->start_mtx);
    pthread_cond_broadcast(&monitor->start_cond);
    // pthread_mutex_unlock(&monitor->start_mtx);
    return SUCCESS;
}

error_t get_done_coders(void **args)
{
    monitor_t   *monitor;
    uint32_t    i;

    monitor = (monitor_t *)args[0];
    i = 0;
    while (i < monitor->nb_coders)
    {
        if (monitor->coders_done[i] && ++i)
            continue;
        if (pthread_mutex_lock(monitor->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;
        if (*monitor->can_take_dungles[i] == ENOUTH_COMPILATIONS
            && ++monitor->coders_done_count)
            monitor->coders_done[i] = 1;
        if (pthread_mutex_unlock(monitor->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;
        i++;
    }
    if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (monitor->coders_done_count == monitor->nb_coders)
        monitor->error_code = SIM_STOP;
    if (pthread_mutex_unlock(&monitor->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    // TODO: Make error manager send real errors so this part can juste if cond return SIM_STOP`
    return SUCCESS;
}

error_t set_dies_in(void **args)
{
    monitor_t   *monitor;
    uint64_t    *ms_time;
    uint32_t    *i;

    monitor = (monitor_t *)args[0];
    i = (uint32_t *)args[1];
    ms_time = (uint64_t *)args[2];
    run("Monitor", get_time, (void **)&(get_tm_t){ms_time});
    if (pthread_mutex_lock(monitor->compile_done_times_mtx[*i]) != SUCCESS)
        return PTHREAD_ERROR;
    monitor->dies_in[*i] = monitor->burount_time - (*ms_time - *monitor->compile_done_times[*i]);
    if (pthread_mutex_unlock(monitor->compile_done_times_mtx[*i]) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

error_t coder_died(void **args)
{
    monitor_t   *monitor;
    uint64_t    ms_time;
    uint32_t    i;

    monitor = (monitor_t *)args[0];
    i = 0;
    while (i < monitor->nb_coders)
    {
        if (monitor->coders_done[i] && ++i)
            continue;
        run("Monitor", set_dies_in, (void *[]){monitor, &i, &ms_time});
        if (monitor->dies_in[i] < 0)
        {
            if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
                return PTHREAD_ERROR;
            monitor->error_code = CODER_DIED;
            print("%i Coder %i died !", (uint32_t []){i, (uint32_t)ms_time - monitor->start}, 2);
            if (pthread_mutex_unlock(&monitor->error_stop_mtx) != SUCCESS)
                return PTHREAD_ERROR;
        }
        else if ((uint64_t)monitor->dies_in[i] < monitor->min_t)
            monitor->min_t = monitor->dies_in[i];
        i++;
    }
    return SUCCESS;
}


error_t wake_coders(void **args)
{
    uint32_t    i;
    monitor_t   *monitor;
    
    monitor = (monitor_t *)args[0];
    i = 0;
    while (i < monitor->nb_coders)
    {
        if (pthread_mutex_lock(monitor->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;
        *monitor->can_take_dungles[i] = (uint8_t)SIM_STOP;
        if (pthread_cond_broadcast(monitor->can_take_cond[i]) != SUCCESS)
            return PTHREAD_ERROR;
        if (pthread_mutex_unlock(monitor->can_take_mtx[i]) != SUCCESS)
            return PTHREAD_ERROR;
        i++;
    }
    return SUCCESS;
}

error_t set_stop(void **args)
{
    monitor_t   *monitor;
    uint8_t     *stop;

    monitor = (monitor_t *)args[0];
    stop = (uint8_t *)args[1];
    if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    if (monitor->error_code != SUCCESS)
    {
        run("Monitor", wake_coders, (void *[]){monitor});
        if (pthread_cond_broadcast(monitor->wake_shedu_cond) != SUCCESS)
            return PTHREAD_ERROR;
        *stop = 1;
    }
    if (pthread_mutex_unlock(&monitor->error_stop_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

uint8_t check_stop(monitor_t *monitor)
{
    uint8_t stop;

    monitor->min_t = monitor->burount_time;
    run("Monitor", get_done_coders, (void *[]){monitor});
    run("Monitor", coder_died, (void *[]){monitor});
    stop = 0;
    run("Monitor", set_stop, (void *[]){monitor, &stop});
    return stop;
}

error_t wait_next(void **args)
{
    monitor_t       *monitor;
    struct timespec ts;
    int             err;

    monitor = (monitor_t *)args[0];
    run("Monitor", ms_to_abstime, (void *[]){&ts, &monitor->min_t});
    if (pthread_mutex_lock(&monitor->wake_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    err = pthread_cond_timedwait(&monitor->wake_cond, &monitor->wake_mtx, &ts);
    if (err != SUCCESS && err != ETIMEDOUT)
        return PTHREAD_ERROR;
    if (pthread_mutex_unlock(&monitor->wake_mtx) != SUCCESS)
        return PTHREAD_ERROR;
    return SUCCESS;
}

void *monitor_runtime(void *arg)
{
    monitor_t   *monitor;

    monitor = (monitor_t *)arg;
    run("Monitor", start, (void **)monitor);
    monitor->coders_done_count = 0;
    while (1)
    {
        if (check_stop(monitor))
            return NULL;
        run("Monitor", wait_next, (void *[]){monitor});
    }
}


#include "codexion.h"

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
                pthread_mutex_unlock(monitor->can_take_mtx[i]);

                i++;
                continue;
            }
            pthread_mutex_unlock(monitor->can_take_mtx[i]);
            run("Monitor", get_time, (void **)&(get_tm_t){&ms_time});
            pthread_mutex_lock(monitor->compile_done_times_mtx[i]);
            if (*monitor->compile_done_times[i] != 0
                && ms_time - *monitor->compile_done_times[i] >= monitor->burount_time)
            {
                pthread_mutex_lock(&monitor->error_stop_mtx);
                monitor->error_code = CODER_DIED;
                print("coder %i died at %i !", (uint32_t []){i, (uint32_t)ms_time - monitor->start}, 2);
                pthread_mutex_unlock(&monitor->error_stop_mtx);
            }
            else
            {
                monitor->dies_in[i] = monitor->burount_time - (ms_time - *monitor->compile_done_times[i]);
                if (monitor->dies_in[i] < monitor->min_t)
                    monitor->min_t = monitor->dies_in[i];
            }
            pthread_mutex_unlock(monitor->compile_done_times_mtx[i]);
            i++;
        }
        // stop if everyone finished
        pthread_mutex_lock(&monitor->error_stop_mtx);
        if (coder_done_count == monitor->nb_coders)
            monitor->error_code = SIM_STOP;
        pthread_mutex_unlock(&monitor->error_stop_mtx);

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
            pthread_cond_broadcast(monitor->wake_shedu_cond);
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


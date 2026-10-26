#include "codexion.h"


void *coder_runtime(void *arg)
{
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
            return NULL;
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
            // pthread_mutex_unlock(&coder->can_take_mtx);
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
            // pthread_mutex_unlock(&coder->can_take_mtx);
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
        
        coder->compile_count++;
        

        // set the new time of not compiling
        run("Coder", get_time, (void **)&(get_tm_t){&ms_time});
        
        pthread_mutex_lock(&coder->compile_done_time_mtx);
        coder->compile_done_time = ms_time;
        pthread_mutex_unlock(&coder->compile_done_time_mtx);
        pthread_mutex_lock(coder->wake_monit_mtx);
        pthread_cond_broadcast(coder->wake_monit_cond);
        pthread_mutex_unlock(coder->wake_monit_mtx);

        usleep(coder->waiting_times[COMPILING] * 1000);
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
#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include "errors.h"

# define DUNGLE_UNUSED ((uint32_t)-1)
# define FREE_SPACE ((uint32_t)-1)

enum can_take_e {
    // managed by coder
    NOT_ASKING,
    USING,
    WAITING,
    STARTUP,
    // managed by sheduler
    CAN_TAKE_DUNGLE,
    // managed by monitor
    ENOUTH_COMPILATIONS,
    SIM_STOP,
};

enum waiting_time_indexs {
    COMPILING = 0,
    DEBUGING = 1,
    REFACTORING = 2
};

typedef struct coder_s {
    int             id;

    // thread
    pthread_t       coder_thread;

    uint8_t         can_take_dungles;
    pthread_mutex_t can_take_mtx;
    pthread_cond_t  can_take_cond;
    
    uint32_t        used_dungle[2];

    pthread_mutex_t compile_done_time_mtx;
    pthread_mutex_t compile_done_time_cond;
    uint64_t        compile_done_time;

    uint32_t        demanded_compilations;
    uint32_t        compile_count;

    pthread_mutex_t *wake_monit_mtx;
    pthread_cond_t  *wake_monit_cond;

    pthread_mutex_t *wake_shedu_mtx;
    pthread_cond_t  *wake_shedu_cond;

    pthread_mutex_t ask_time_mtx;
    uint64_t        ask_time;

    uint32_t        dungles[2];

    uint32_t        waiting_times[3];


    pthread_mutex_t *error_stop_mtx;
    uint8_t         *error_code;

    

    pthread_mutex_t *start_mtx;
    pthread_cond_t  *start_cond;

    uint64_t        *start;
    struct timeval  tv;
}   coder_t;


typedef struct sheduler_s {
    uint32_t        nb_coders;

    // thread
    pthread_t shedulter_thread;

    // verify who is asking for dungles and give access to dungles
    uint8_t         **can_take_dungles;
    pthread_mutex_t **can_take_mtx;
    pthread_cond_t  **can_take_cond;
        // optimisation
    pthread_mutex_t wake_mtx;
    pthread_cond_t  wake_cond;

    // verify the priority of the dungles
    // input
    pthread_mutex_t **ask_times_mtx;
    uint64_t        **ask_times;

    pthread_mutex_t **compile_done_times_mtx;
    uint64_t        **compile_done_times;

    uint64_t        dungle_time;
    // output
    uint32_t        (*dungles_priority)[2];

    // set a deadlock free dungle assignement
    uint32_t        *dungles;

    // stop simulation
    pthread_mutex_t *error_stop_mtx;
    uint8_t         *error_code;

    // time relevent structure

    pthread_mutex_t *start_mtx;
    pthread_cond_t  *start_cond;

    uint64_t        *start;
    struct timeval  tv;
}   sheduler_t;

typedef struct monitor_s {
    uint32_t        nb_coders;
    pthread_t       thread;

    uint32_t        burount_time;
    uint64_t        start;

    uint64_t        *dies_in;
    uint32_t        min_t;
    
    pthread_mutex_t wake_mtx;
    pthread_cond_t  wake_cond;

    pthread_mutex_t *wake_shedu_mtx;
    pthread_cond_t  *wake_shedu_cond;

    pthread_mutex_t **compile_done_times_mtx;
    uint64_t        **compile_done_times;

    pthread_mutex_t error_stop_mtx;
    uint8_t         error_code;

    uint8_t         **can_take_dungles;
    pthread_mutex_t **can_take_mtx;
    pthread_cond_t  **can_take_cond;

    pthread_mutex_t start_mtx;
    pthread_cond_t  start_cond;

    struct timeval  tv;
} monitor_t;

typedef struct codexion_s {
    uint32_t        nb_coders;
    coder_t         *coders;
    sheduler_t      sheduler;
    monitor_t       monitor;
    errors_t        *err_manager;
}   codexion_t;


typedef struct get_time_s {
    uint64_t        *ms_time;
} get_tm_t;

error_t get_time(void **args);
void ms_to_abstime(struct timespec *ts, uint64_t add_ms);
void *sheduler_runtime(void *arg);
void *monitor_runtime(void *arg);
void *coder_runtime(void *arg);

#endif
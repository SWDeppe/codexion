#ifndef ERRORS_H
# define ERRORS_H

#include <stdint.h>
#include <pthread.h>
typedef uint8_t error_t;

# ifndef SCREEN_SIZE_W
#  define SCREEN_SIZE_W 70
# endif

# ifndef SCREEN_SIZE_H
#  define SCREEN_SIZE_H 40
# endif

enum error_e {
    SUCCESS = 0,
    MISSING_NB_CODERS = 1,
    MISSING_BOURNOUT_TIME = 2,
    MISSING_COMPILE_TIME = 3,
    MISSING_DEBUG_TIME = 4,
    MISSING_REFACTOR_TIME = 5,
    MISSING_REQUIRED_COMIPLATION = 6,
    MISSING_DUNGLE_TIME = 7,
    MISSING_SHEDULER_TYPE = 8,
    UNKNOWENED_SHEDULER_TYPE = 9,
    HEAPQ_SIZE_TO_LOW,
    CODER_DIED,
    GET_TIME_OF_DAY_ERROR,
    MALLOC_ERROR,
    ERROR_LIST_END
};


//typedef enum parsing_error_e {
//    SUCCESS = 0,
//    MISSING_NB_CODERS = 1,
//    MISSING_BOURNOUT_TIME = 2,
//    MISSING_COMPILE_TIME = 3,
//    MISSING_DEBUG_TIME = 4,
//    MISSING_REFACTOR_TIME = 5,
//    MISSING_REQUIRED_COMIPLATION = 6,
//    MISSING_DUNGLE_TIME = 7,
//    MISSING_SHEDULER_TYPE = 8,
//    UNKNOWENED_SHEDULER_TYPE = 9,
//} pars_err_t;
//
//typedef enum heapq_error_e {
//    SUCCESS,
//    HEAPQ_SIZE_TO_LOW,
//    MALLOC_ERROR,
//} heap_err_t;
//
//
//typedef enum waiter_error_e {
//    // MALLOC_ERROR,
//    SUCCESS
//} waiter_err_t;



typedef struct error_manager {
    char            **owners;
    uint32_t        owner_count;

    error_t         **owned_errors;
    uint32_t        *owned_errors_length;
    
    char            **logs;
    uint32_t        **logs_args;
    uint32_t        logs_count;
    uint32_t        max_logs;
    
    pthread_t       thread;
    pthread_mutex_t manager_mutex;
} errors_t;

void    run(char *owner, error_t (* func)(void **), void **args);
void    *manager_runtime(void *arg);
void    error_table(errors_t *manager);
void    print(char *msg, uint32_t *args, uint32_t args_count);

#endif

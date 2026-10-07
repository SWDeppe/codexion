#ifndef ERRORS_H
# define ERRORS_H

typedef enum error_e {
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
    MALLOC_ERROR,
} error_t;


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

#endif
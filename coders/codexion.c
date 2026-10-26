#include "codexion.h"
#include "parser.h"
#include "heapq.h"
#include "realloc.h"
#include "errors.h"


error_t get_time(void **args)
{
    get_tm_t    *vars;
    struct timeval tv;

    vars = (get_tm_t *)args;
    if (gettimeofday(&tv, NULL) != 0)
        return GET_TIME_OF_DAY_ERROR;
    *vars->ms_time = tv.tv_usec / 1000 + tv.tv_sec * 1000;
    return SUCCESS;
}


void ms_to_abstime(struct timespec *ts, uint64_t add_ms)
{
    struct timeval  tv;
    uint64_t        us;

    gettimeofday(&tv, NULL);
    us = (uint64_t)tv.tv_usec + (add_ms % 1000) * 1000;
    ts->tv_sec = tv.tv_sec + add_ms / 1000 + us / 1000000;
    ts->tv_nsec = (us % 1000000) * 1000;
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
    codexion->monitor.wake_shedu_cond = &codexion->sheduler.wake_cond;
    codexion->monitor.wake_shedu_mtx = &codexion->sheduler.wake_mtx;
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
    pthread_create(&codexion->sheduler.shedulter_thread, NULL, sheduler_runtime, &codexion->sheduler);
    pthread_create(&codexion->monitor.thread, NULL, monitor_runtime, &codexion->monitor);
    while (i < codexion->nb_coders)
    {
        pthread_create(&codexion->coders[i].coder_thread, NULL, coder_runtime, &codexion->coders[i]);
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

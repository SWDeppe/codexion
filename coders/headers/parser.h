#ifndef PARSER_H
# define PARSER_H

# include "errors.h"
# include "string.h"
# include "stdlib.h"
# include "unistd.h"

typedef enum shedule_e {
    FIFO,
    EDF
} shedule_t;


typedef struct parsingv_s {
    unsigned int    nb_coders;
    unsigned int    burnout_time;
    unsigned int    compile_time;
    unsigned int    debug_time;
    unsigned int    refactor_time;
    unsigned int    required_compil;
    unsigned int    dungle_time;
    shedule_t       sheduler;
} pars_t;

error_t  parse(int argc, char **argv, pars_t *pars_values);
void    send_error(error_t error);

#endif
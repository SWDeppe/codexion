#include "parser.h"
#include "errors.h"

void    send_error(error_t error) {
    if (error == MISSING_NB_CODERS)
        write(2, "Mising nb coders parameter", 26);
    else if (error == MISSING_BOURNOUT_TIME)
        write(2, "Mising burnout time parameter", 29);
    else if (error == MISSING_COMPILE_TIME)
        write(2, "Mising compile time parameter", 29);
    else if (error == MISSING_DEBUG_TIME)
        write(2, "Mising debug time parameter", 27);
    else if (error == MISSING_REFACTOR_TIME)
        write(2, "Mising refactore time parameter", 31);
    else if (error == MISSING_REQUIRED_COMIPLATION)
        write(2, "Mising required compilation parameter", 37);
    else if (error == MISSING_DUNGLE_TIME)
        write(2, "Mising dungle time parameter", 28);
    else if (error == MISSING_SHEDULER_TYPE)
        write(2, "Mising sheduler type parameter", 30);
    else if (error == UNKNOWENED_SHEDULER_TYPE)
        write(2, "Unknowened sheduler type parameter", 34);
}

error_t  parse(int argc, char **argv, pars_t *pars_values) {
    if (argc < 9)
        return argc - 1;
    pars_values->nb_coders = (unsigned int)atoi(argv[1]);
    pars_values->burnout_time = (unsigned int)atoi(argv[2]);
    pars_values->compile_time = (unsigned int)atoi(argv[3]);
    pars_values->debug_time = (unsigned int)atoi(argv[4]);
    pars_values->refactor_time = (unsigned int)atoi(argv[5]);
    pars_values->required_compil = (unsigned int)atoi(argv[6]);
    pars_values->dungle_time = (unsigned int)atoi(argv[7]);
    if (strcmp(argv[8], "FIFO") == 0)
        pars_values->sheduler = FIFO;
    else if (strcmp(argv[8], "EDF") == 0)
        pars_values->sheduler = EDF;
    else
        return UNKNOWENED_SHEDULER_TYPE;
    return SUCCESS;
}

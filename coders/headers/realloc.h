#ifndef REALLOC_H
# define REALLOC_H

#include "stdlib.h"
#include "stdint.h"

void    *my_realloc(void *ptr, uint32_t old_size, uint32_t new_size);

#endif
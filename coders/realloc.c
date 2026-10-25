#include "realloc.h"

void    *my_realloc(void *ptr, uint32_t old_size, uint32_t new_size)
{
    void        *ret;
    uint32_t    i;

    if (old_size > new_size)
        return NULL;
    ret = (void *)malloc(sizeof(char) * new_size);
    i = 0;
    while (i < old_size)
    {
        ((char *)ret)[i] = ((char *)ptr)[i];
        i++;
    }
    free(ptr);
    return ret;
}

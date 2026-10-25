#include "heapq.h"
#include "realloc.h"

// Min-heap on score; pop returns the id of the lowest score
error_t init_heapq(void **args) {
    int         i;
    i_heapq_t   *vars;

    vars = (i_heapq_t *)args;
    if (vars->max_size < (uint32_t)vars->base_size)
        vars->max_size = vars->base_size;
    vars->heapq->heap_lst = (heap_node_t *)malloc(sizeof(heap_node_t) * vars->max_size);
    if (!vars->heapq->heap_lst)
        return MALLOC_ERROR;
    vars->heapq->size = 0;
    vars->heapq->max_size = vars->max_size;
    i = 0;
    while (vars->heapq->heap_lst && i < vars->base_size) {
        heapq_insert(vars->heapq, vars->base_scores[i], vars->base_ids[i]);
        i++;
    }
    return SUCCESS;
}

uint32_t    pop(heapq_t *heapq) {
    uint32_t    ret;
    uint32_t    i;
    uint32_t    smallest;

    if (heapq->size == 0)
        return (uint32_t)-1;
    ret = heapq->heap_lst[0].id;
    heapq->heap_lst[0] = heapq->heap_lst[--heapq->size];
    i = 0;
    while (1)
    {
        smallest = i;
        if (get_left(i) < heapq->size &&
            heapq->heap_lst[get_left(i)].score < heapq->heap_lst[smallest].score)
            smallest = get_left(i);
        if (get_right(i) < heapq->size &&
            heapq->heap_lst[get_right(i)].score < heapq->heap_lst[smallest].score)
            smallest = get_right(i);
        if (smallest == i)
            break;
        swap(&heapq->heap_lst[smallest], &heapq->heap_lst[i]);
        i = smallest;
    }
    return ret;
}

error_t resize_heapq(heapq_t *heapq) {
    heap_node_t *tmp;
    uint32_t    new_size;

    if (heapq->max_size != 0)
        new_size = heapq->max_size * 2;
    else
        new_size = 2;
    tmp = (heap_node_t *)my_realloc(heapq->heap_lst,
                                    sizeof(heap_node_t) * heapq->size,
                                    sizeof(heap_node_t) * new_size);
    if (!tmp)
        return MALLOC_ERROR;
    heapq->heap_lst = tmp;
    heapq->max_size = new_size;
    return SUCCESS;
}

error_t heapq_insert(heapq_t *heapq, uint64_t score, uint32_t id) {
    uint32_t    i;

    if (heapq->size >= heapq->max_size)
        if (resize_heapq(heapq) != SUCCESS)
            return MALLOC_ERROR;
    // printf("score of 0: %lu\n", heapq->heap_lst[0].score);
    // printf("score of 0: %lu\n", heapq->heap_lst[0].score);
    i = heapq->size++;
    heapq->heap_lst[i].score = score;
    heapq->heap_lst[i].id = id;
    while (i > 0 && heapq->heap_lst[get_parent(i)].score > heapq->heap_lst[i].score) {
        swap(&heapq->heap_lst[get_parent(i)], &heapq->heap_lst[i]);
        i = get_parent(i);
    }
    return SUCCESS;
}

void heapq_destroy(heapq_t *heapq) {
    free(heapq->heap_lst);
}
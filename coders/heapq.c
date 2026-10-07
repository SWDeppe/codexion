#include "heapq.h"

// Min-heap on score; pop returns the id of the lowest score
heapq_t init_heapq(int base_size, uint32_t *base_scores, uint32_t *base_ids, uint32_t max_size) {
    heapq_t     heapq;
    int         i;

    if (max_size < (uint32_t)base_size)
        max_size = base_size;
    heapq.heap_lst = (heap_node_t *)malloc(sizeof(heap_node_t) * max_size);
    heapq.size = 0;
    heapq.max_size = heapq.heap_lst ? max_size : 0;
    i = 0;
    while (heapq.heap_lst && i < base_size) {
        heapq_insert(&heapq, base_scores[i], base_ids[i]);
        i++;
    }
    return heapq;
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

    new_size = heapq->max_size ? heapq->max_size * 2 : 2;
    tmp = (heap_node_t *)realloc(heapq->heap_lst, sizeof(heap_node_t) * new_size); // TODO use malloc
    if (!tmp)
        return MALLOC_ERROR;
    heapq->heap_lst = tmp;
    heapq->max_size = new_size;
    return SUCCESS;
}

error_t heapq_insert(heapq_t *heapq, uint32_t score, uint32_t id) {
    uint32_t    i;

    if (heapq->size >= heapq->max_size)
        if (resize_heapq(heapq) != SUCCESS)
            return MALLOC_ERROR;
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
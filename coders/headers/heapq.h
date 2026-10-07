#ifndef HEAPQ_H
# define HEAPQ_H

# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
// # define DEBUG // TODO: juste during compilation
// # ifdef DEBUG
# include <assert.h>
// # endif
# include "errors.h"

typedef struct heap_node_s {
    uint32_t score;
    uint32_t id;
} heap_node_t;

typedef struct heapq_s {
    uint32_t size;
    heap_node_t *heap_lst;
    uint32_t max_size;
} heapq_t;

void swap(heap_node_t *nb1, heap_node_t *nb2);
error_t  heapq_insert(heapq_t *heapq, uint32_t score, uint32_t id);
error_t  resize_heapq(heapq_t *heapq);
uint32_t    pop(heapq_t *heapq);
heapq_t init_heapq(int base_size, uint32_t *base_scores, uint32_t *base_ids, uint32_t max_size);
uint32_t get_parent(uint32_t child_index);
uint32_t get_left(uint32_t parent);
uint32_t get_right(uint32_t parent);
void heapq_destroy(heapq_t *heapq);

#endif
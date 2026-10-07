#include "heapq.h"

uint32_t get_parent(uint32_t child_index) {
    if (child_index == 0)
        return 0;
    return (uint32_t)(child_index - 1) / 2;
}

uint32_t get_left(uint32_t parent) {
    return parent * 2 + 1;
}

uint32_t get_right(uint32_t parent) {
    return parent * 2 + 2;
}

void swap(heap_node_t *nb1, heap_node_t *nb2) {
    heap_node_t tmp;

    tmp = *nb1;
    *nb1 = *nb2;
    *nb2 = tmp;
}

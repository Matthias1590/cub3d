#pragma once

#include <stddef.h>

void mem_cpy(void *dest, const void *src, size_t size);
void *mem_realloc(void *old_ptr, size_t old_size, size_t new_size);
void mem_swap(void *m1, void *m2, size_t size);

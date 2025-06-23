#include "mem.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

void mem_cpy(void *dest, const void *src, size_t size)
{
	size_t i = 0;
	while (i < size)
	{
		((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
		i++;
	}
}

void *mem_realloc(void *old_ptr, size_t old_size, size_t new_size)
{
	assert(new_size >= old_size);  // TODO: Handle new_size < old_size

	void *new_ptr = malloc(new_size);
	if (!new_ptr)
		return (NULL);
	mem_cpy(new_ptr, old_ptr, old_size);
	free(old_ptr);
	return (new_ptr);
}

void mem_swap(void *m1, void *m2, size_t size)
{
	size_t i = 0;
	while (i < size)
	{
		uint8_t temp = ((uint8_t *)m1)[i];
		((uint8_t *)m1)[i] = ((uint8_t *)m2)[i];
		((uint8_t *)m2)[i] = temp;
		i++;
	}
}

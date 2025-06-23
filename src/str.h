#pragma once

#include "pool.h"

size_t str_len(const char *s);
char *str_dup(pool_t *pool, const char *s);
char *str_add(pool_t *pool, char *s1, const char *s2);

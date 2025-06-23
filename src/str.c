#include "str.h"
#include "mem.h"
#include "pool.h"

size_t str_len(const char *s)
{
	size_t i = 0;
	while (s[i])
		i++;
	return (i);
}

char *str_dup(pool_t *pool, const char *s)
{
	size_t len = str_len(s);
	char *dup = pool_alloc(pool, len + 1);
	if (!dup)
		return (NULL);
	mem_cpy(dup, s, len);
	dup[len] = '\0';
	return (dup);
}

char *str_add(pool_t *pool, char *s1, const char *s2)
{
	size_t len = str_len(s1) + str_len(s2);
	char *added = pool_alloc(pool, len + 1);
	if (!added)
		return (NULL);
	mem_cpy(added, s1, str_len(s1));
	mem_cpy(added + str_len(s1), s2, str_len(s2));
	pool_free(pool, s1);
	added[len] = '\0';
	return (added);
}

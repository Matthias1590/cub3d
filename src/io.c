#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "io.h"
#include "str.h"

char *read_entire_file(pool_t *pool, const char *path)
{
	pool_t *local_pool = pool_create();
	if (!local_pool)
		return (NULL);

	int fd = pool_open(local_pool, path, O_RDONLY);
	if (fd < 0)
		return (pool_destroy(local_pool), NULL);
	
	char buf[10] = {0};
	char *content = str_dup(local_pool, "");
	while (true)
	{
		ssize_t res = read(fd, buf, sizeof(buf) - 1);
		if (res < 0)
			return (pool_destroy(local_pool), NULL);
		if (res == 0)
			break ;
		buf[res] = '\0';
		content = str_add(local_pool, content, buf);
		if (!content)
			return (pool_destroy(local_pool), NULL);
	}

	return (pool_return(local_pool, content, pool));
}

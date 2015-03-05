#include "pipefs_cache.h"

#include "Cache.hpp"
#include "ReadLoop.hpp"

#include <memory>

extern "C" {

struct pipefs_cache {
	int dummy;
};

struct pipefs_cache* pipefs_cache_create()
{
	return reinterpret_cast<pipefs_cache*>(new pipefs::Cache{});
}

void pipefs_cache_destroy(struct pipefs_cache* cache)
{
	delete reinterpret_cast<const pipefs::Cache*>(cache);
}

int pipefs_cache_read(const struct pipefs_cache* cache, void* buf,
		size_t length, size_t position)
{
	return reinterpret_cast<const pipefs::Cache*>(cache)->read(buf, length, position);
}

struct pipefs_readloop {
	int dummy;
};

struct pipefs_readloop* pipefs_readloop_create()
{
	return reinterpret_cast<pipefs_readloop*>(new pipefs::ReadLoop{});
}

void pipefs_readloop_destroy(struct pipefs_readloop* readloop)
{
	delete reinterpret_cast<pipefs::ReadLoop*>(readloop);
}


void pipefs_readloop_start(struct pipefs_readloop* readloop)
{
	reinterpret_cast<pipefs::ReadLoop*>(readloop)->start();
}

void pipefs_readloop_stop(struct pipefs_readloop* readloop)
{
	reinterpret_cast<pipefs::ReadLoop*>(readloop)->stop();
}

void pipefs_readloop_add(struct pipefs_readloop* readloop, int fd,
		struct pipefs_cache* cache)
{
	reinterpret_cast<pipefs::ReadLoop*>(readloop)->add(fd, *reinterpret_cast<pipefs::Cache*>(cache));
}

void pipefs_readloop_remove(struct pipefs_readloop* readloop, int fd)
{
	reinterpret_cast<pipefs::ReadLoop*>(readloop)->remove(fd);
}

}


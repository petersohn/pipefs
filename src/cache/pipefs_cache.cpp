#include "pipefs_cache.h"

#include "Cache.hpp"
#include "ReadLoop.hpp"

#include <memory>

extern "C" {

struct pipefs_cache {
	pipefs::Cache cache;
};

struct pipefs_cache* pipefs_cache_create()
{
	return new pipefs_cache{};
}

void pipefs_cache_destroy(struct pipefs_cache* cache)
{
	delete cache;
}

int pipefs_cache_read(const struct pipefs_cache* cache, void* buf,
		size_t length, size_t position)
{
	return cache->cache.read(buf, length, position);
}

struct pipefs_readloop {
	pipefs::ReadLoop readLoop;
};

struct pipefs_readloop* pipefs_readloop_create()
{
	return new pipefs_readloop{};
}

void pipefs_readloop_destroy(struct pipefs_readloop* readloop)
{
	delete readloop;
}


void pipefs_readloop_start(struct pipefs_readloop* readloop)
{
	readloop->readLoop.start();
}

void pipefs_readloop_stop(struct pipefs_readloop* readloop)
{
	readloop->readLoop.stop();
}

void pipefs_readloop_add(struct pipefs_readloop* readloop, int fd, 
		struct pipefs_cache* cache)
{
	readloop->readLoop.add(fd, cache->cache);
}

void pipefs_readloop_remove(struct pipefs_readloop* readloop, int fd)
{
	readloop->readLoop.remove(fd);
}

}


#ifndef SRC_CACHE_PIPEFS_CACHE_H
#define SRC_CACHE_PIPEFS_CACHE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pipefs_cache;

struct pipefs_cache* pipefs_cache_create();
void pipefs_cache_destroy(struct pipefs_cache* cache);
int pipefs_cache_read(const struct pipefs_cache* cache, void* buf,
		size_t length, size_t position);

struct pipefs_caches;

struct pipefs_caches* pipefs_caches_create();
void pipefs_caches_destroy(struct pipefs_caches* caches);
int pipefs_caches_get(struct pipefs_caches* caches, const char* key,
		struct pipefs_cache** cache);
void pipefs_caches_release(struct pipefs_caches* caches, const char* key);
void pipefs_caches_cleanup(struct pipefs_caches* caches, size_t target_size);

struct pipefs_readloop;

struct pipefs_readloop* pipefs_readloop_create();
void pipefs_readloop_destroy(struct pipefs_readloop* readloop);
void pipefs_readloop_start(struct pipefs_readloop* readloop);
void pipefs_readloop_stop(struct pipefs_readloop* readloop);
void pipefs_readloop_add(struct pipefs_readloop* readloop, int fd,
		struct pipefs_cache* cache);
void pipefs_readloop_remove(struct pipefs_readloop* readloop, int fd);

#ifdef __cplusplus
}
#endif



#endif /* SRC_CACHE_PIPEFS_CACHE_H */

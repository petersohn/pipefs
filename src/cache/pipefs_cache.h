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

struct pipefs_io_thread;

struct pipefs_io_thread* pipefs_io_thread_create();
void pipefs_io_thread_destroy(struct pipefs_io_thread* io_thread);
void pipefs_io_thread_start(struct pipefs_io_thread* io_thread);
void pipefs_io_thread_stop(struct pipefs_io_thread* io_thread);

struct pipefs_readloop;
typedef int(*read_starter)(void*);

struct pipefs_readloop* pipefs_readloop_create(struct pipefs_io_thread* io_thread);
void pipefs_readloop_destroy(struct pipefs_readloop* readloop);
void pipefs_readloop_cancel(struct pipefs_readloop* readloop);
void pipefs_readloop_add(struct pipefs_readloop* readloop, read_starter starter,
		struct pipefs_cache* cache, void* data);
void pipefs_readloop_remove(struct pipefs_readloop* readloop, int fd);

struct pipefs_signal_handler;

struct pipefs_signal_handler* pipefs_signal_handler_create(struct pipefs_io_thread* io_thread);
void pipefs_signal_handler_destroy(struct pipefs_signal_handler* signal_handler);
void pipefs_signal_handler_start(struct pipefs_signal_handler* signal_handler);
void pipefs_signal_handler_cancel(struct pipefs_signal_handler* signal_handler);

#ifdef __cplusplus
}
#endif



#endif /* SRC_CACHE_PIPEFS_CACHE_H */

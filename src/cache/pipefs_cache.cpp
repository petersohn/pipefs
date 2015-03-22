#include "pipefs_cache.h"
#include "log.h"

#include "Cache.hpp"
#include "Caches.hpp"
#include "ReadLoop.hpp"
#include "SignalHandler.hpp"
#include "IOThread.hpp"

#include <boost/exception/all.hpp>
#include <memory>

extern "C" {


#define TRY(expr) try { expr; } catch (std::exception& e) { \
	log_msg("%s\n", boost::diagnostic_information(e).c_str()); \
	throw; \
}


struct pipefs_cache* pipefs_cache_create()
{
	TRY(return reinterpret_cast<pipefs_cache*>(new pipefs::Cache{}));
}

void pipefs_cache_destroy(struct pipefs_cache* cache)
{
	TRY(delete reinterpret_cast<const pipefs::Cache*>(cache));
}

int pipefs_cache_read(const struct pipefs_cache* cache, void* buf,
		size_t length, size_t position)
{
	TRY(return reinterpret_cast<const pipefs::Cache*>(cache)->read(buf, length, position));
}


struct pipefs_caches* pipefs_caches_create()
{
	TRY(return reinterpret_cast<pipefs_caches*>(new pipefs::Caches{}));
}

void pipefs_caches_destroy(struct pipefs_caches* caches)
{
	TRY(delete reinterpret_cast<pipefs::Caches*>(caches));
}

int pipefs_caches_get(struct pipefs_caches* caches, const char* key,
		struct pipefs_cache** cache)
{
	TRY(
		auto result = reinterpret_cast<pipefs::Caches*>(caches)->get(key);
		if (cache) {
			*cache = reinterpret_cast<pipefs_cache*>(&result.first);
		}
		return result.second;
	)
}

void pipefs_caches_release(struct pipefs_caches* caches, const char* key)
{
	TRY(reinterpret_cast<pipefs::Caches*>(caches)->release(key));
}

void pipefs_caches_cleanup(pipefs_caches* caches, size_t target_size)
{
	TRY(reinterpret_cast<pipefs::Caches*>(caches)->cleanup(target_size));
}

struct pipefs_io_thread* pipefs_io_thread_create()
{
	TRY(return reinterpret_cast<pipefs_io_thread*>(new pipefs::IOThread{}));
}

void pipefs_io_thread_destroy(struct pipefs_io_thread* io_thread)
{
	TRY(delete reinterpret_cast<pipefs::IOThread*>(io_thread));
}

void pipefs_io_thread_start(struct pipefs_io_thread* io_thread)
{
	TRY(reinterpret_cast<pipefs::IOThread*>(io_thread)->start());
}

void pipefs_io_thread_stop(struct pipefs_io_thread* io_thread)
{
	TRY(reinterpret_cast<pipefs::IOThread*>(io_thread)->stop());
}


struct pipefs_readloop* pipefs_readloop_create(struct pipefs_io_thread* io_thread)
{
	TRY(return reinterpret_cast<pipefs_readloop*>(new pipefs::ReadLoop{
			reinterpret_cast<pipefs::IOThread*>(io_thread)->getIoService()}));
}

void pipefs_readloop_destroy(struct pipefs_readloop* readloop)
{
	TRY(delete reinterpret_cast<pipefs::ReadLoop*>(readloop));
}

void pipefs_readloop_cancel(struct pipefs_readloop* readloop)
{
	TRY(reinterpret_cast<pipefs::ReadLoop*>(readloop)->cancel());
}

void pipefs_readloop_add(struct pipefs_readloop* readloop,
		read_starter starter, struct pipefs_cache* cache, void* data)
{
	pipefs::ReadLoop::ReadStarter readStarter = [starter, data](
			boost::asio::io_service& ioService)
		{
			return std::make_shared<boost::asio::posix::stream_descriptor>(
					ioService, starter(data));
		};
	TRY(reinterpret_cast<pipefs::ReadLoop*>(readloop)->add(readStarter,
			*reinterpret_cast<pipefs::Cache*>(cache)));
}

void pipefs_readloop_remove(struct pipefs_readloop* readloop, int fd)
{
	TRY(reinterpret_cast<pipefs::ReadLoop*>(readloop)->remove(fd));
}


struct pipefs_signal_handler* pipefs_signal_handler_create(struct pipefs_io_thread* io_thread)
{
	TRY(return reinterpret_cast<pipefs_signal_handler*>(new pipefs::SignalHandler{
			reinterpret_cast<pipefs::IOThread*>(io_thread)->getIoService()}));
}

void pipefs_signal_handler_destroy(struct pipefs_signal_handler* signal_handler)
{
	TRY(delete reinterpret_cast<pipefs::SignalHandler*>(signal_handler));
}

void pipefs_signal_handler_cancel(struct pipefs_signal_handler* signal_handler)
{
	TRY(reinterpret_cast<pipefs::SignalHandler*>(signal_handler)->cancel());
}

void pipefs_signal_handler_start(struct pipefs_signal_handler* signal_handler)
{
	TRY(reinterpret_cast<pipefs::SignalHandler*>(signal_handler)->start());
}

}


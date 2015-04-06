#ifndef SRC_CACHE_BASICREADLOOP_HPP
#define SRC_CACHE_BASICREADLOOP_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

#include "log.h"

namespace pipefs {

template <typename StreamDescriptor, typename Cache, typename Logger>
class BasicReadLoop {
public:
	using ReadStarter = std::function<std::shared_ptr<StreamDescriptor>(
			boost::asio::io_service&)>;

	BasicReadLoop(boost::asio::io_service& ioService, Logger logger = Logger{}):
			ioService(ioService), logger(std::move(logger)) {}

	void cancel()
	{
		logger("ReadLoop::cancel()\n");
		ioService.post([this]() {
			logger("ReadLoop::doCancel()\n");
			caches.clear();
		});
	}

	void add(ReadStarter readStarter, std::shared_ptr<Cache> cache)
	{
		auto stream = readStarter(ioService);
		assert(stream);
		auto fd = stream->native_handle();
		logger("ReadLoop::add(%d)\n", fd);

		ioService.post([this, cache, stream, fd]() {
			logger("ReadLoop::doAdd(%d)\n", fd);
			auto emplaceResult = caches.emplace(fd,
					CacheData{logger, cache, stream, ""});

			if (emplaceResult.second) {
				logger("  added to read loop\n");
				auto& data = emplaceResult.first->second;
				startReading(data.stream, data.cache, data.buffer);
			} else {
				logger("  not added to read loop\n");
			}
		});
	}

	void remove(int fd)
	{
		logger("ReadLoop::remove(%d)\n", fd);

		ioService.post([this, fd]() {
			logger("ReadLoop::doRemove(%d)\n", fd);
			auto result = caches.erase(fd);
			logger("  result = %lu\n", result);
		});
	}

private:
	constexpr static std::size_t bufferSize = 65536;

	boost::asio::io_service& ioService;
	Logger logger;

	struct CacheData {
		Logger& logger;
		std::shared_ptr<Cache> cache;
		std::shared_ptr<StreamDescriptor> stream;
		char buffer[bufferSize];

		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;

		~CacheData()
		{
			if (stream) {
				logger("  remove cache for fd %d from read loop\n",
						stream->native_handle());
				cache->finish();
				boost::system::error_code errorCode;
				stream->cancel(errorCode);
				if (stream->is_open()) {
					stream->close(errorCode);
					// ignore the error
				}
			}
		}
	};

	std::map<int, CacheData> caches;

	void startReading(
			std::shared_ptr<StreamDescriptor> stream,
			std::shared_ptr<Cache> cache,
			char* buffer)
	{
		logger("ReadLoop::startReading(%d)\n", stream->native_handle());
		using std::placeholders::_1;
		using std::placeholders::_2;
		stream->async_read_some(
				boost::asio::buffer(buffer, bufferSize),
				std::bind(&BasicReadLoop::readFinished, this, stream, cache,
						buffer, _1, _2));
	}

	void readFinished(
			std::shared_ptr<StreamDescriptor> stream,
			std::shared_ptr<Cache> cache,
			char* buffer,
			boost::system::error_code errorCode, std::size_t bytesTransferred)
	{
		logger("ReadLoop::readFinished(fd=%d, error=%s, bytes=%lu)\n",
				stream->native_handle(), errorCode.message().c_str(),
				bytesTransferred);

		if (!errorCode && bytesTransferred > 0 && stream->is_open()) {
			cache->write(buffer, bytesTransferred);
			startReading(stream, cache, buffer);
			return;
		}

		int key = stream->native_handle();
		remove(key);
	}
};

}

#endif /* SRC_CACHE_BASICREADLOOP_HPP */

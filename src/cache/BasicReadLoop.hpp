#ifndef SRC_CACHE_BASICREADLOOP_HPP
#define SRC_CACHE_BASICREADLOOP_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <map>
#include <functional>
#include <memory>

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
				logger("Clear caches.\n");
				caches.clear();
			});
	}

	void add(ReadStarter readStarter, Cache& cache)
	{
		auto stream = readStarter(ioService);
		assert(stream);
		auto fd = stream->native_handle();
		logger("ReadLoop::add(%d)\n", fd);
		auto emplaceResult = caches.emplace(fd,
				CacheData{logger, cache, stream, ""});

		if (emplaceResult.second) {
			logger("  added to read loop\n");
			auto& data = emplaceResult.first->second;
			startReading(data);
		} else {
			logger("  not added to read loop\n");
		}
	}

	void remove(int fd)
	{
		logger("ReadLoop::remove(%d)\n", fd);
		auto result = caches.erase(fd);
		logger("  result = %lu\n", result);
	}

private:
	constexpr static std::size_t bufferSize = 65536;

	boost::asio::io_service& ioService;
	Logger logger;

	struct CacheData {
		Logger& logger;
		Cache& cache;
		std::shared_ptr<StreamDescriptor> stream;
		char buffer[bufferSize];

		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;

		~CacheData()
		{
			if (stream) {
				logger("  remove cache for fd %d from read loop\n",
						stream->native_handle());
				cache.finish();
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

	void startReading(CacheData& data)
	{
		logger("ReadLoop::startReading(%d)\n", data.stream->native_handle());
		using std::placeholders::_1;
		using std::placeholders::_2;
		data.stream->async_read_some(
				boost::asio::buffer<char, bufferSize>(data.buffer),
				std::bind(&BasicReadLoop::readFinished, this, std::ref(data),
					data.stream, _1, _2));
	}

	void readFinished(CacheData& data, std::shared_ptr<StreamDescriptor> stream,
			boost::system::error_code errorCode, std::size_t bytesTransferred)
	{
		logger("ReadLoop::readFinished(fd=%d, error=%s, bytes=%lu)\n",
				stream->native_handle(), errorCode.message().c_str(),
				bytesTransferred);

		if (!errorCode && bytesTransferred > 0 && stream->is_open()) {
			data.cache.write(data.buffer, bytesTransferred);
			startReading(data);
			return;
		}

		int key = stream->native_handle();
		remove(key);
	}
};

}

#endif /* SRC_CACHE_BASICREADLOOP_HPP */

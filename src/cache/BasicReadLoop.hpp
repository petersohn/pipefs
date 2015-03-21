#ifndef SRC_CACHE_BASICREADLOOP_HPP
#define SRC_CACHE_BASICREADLOOP_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <map>
#include <functional>

#include "log.h"

namespace pipefs {

template <typename StreamDescriptor, typename Cache, typename Logger>
class BasicReadLoop {
public:
	using ReadStarter = std::function<int()>;

	BasicReadLoop(boost::asio::io_service& ioService, Logger logger = Logger{}):
			ioService(ioService), logger(std::move(logger)) {}

	void cancel()
	{
		for (auto& value: caches) {
			value.second.stream.cancel();
		}
	}

	void add(ReadStarter readStarter, Cache& cache)
	{
		int fd = readStarter();
		logger("ReadLoop::add(%d)\n", fd);
		auto emplaceResult = caches.emplace(fd,
				CacheData{cache, {ioService, fd}, ""});

		if (emplaceResult.second) {
			auto& data = emplaceResult.first->second;
			startReading(data);
		}
	}

	void remove(int fd)
	{
		logger("ReadLoop::remove(%d)\n", fd);
		auto it = caches.find(fd);
		if (it != caches.end()) {
			auto& data = it->second;
			boost::system::error_code errorCode;
			data.stream.cancel(errorCode);
			// ignore the error

			caches.erase(it);
		}
	}

private:
	constexpr static std::size_t bufferSize = 65536;

	boost::asio::io_service& ioService;
	Logger logger;

	struct CacheData {
		Cache& cache;
		StreamDescriptor stream;
		char buffer[bufferSize];

		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;
	};

	std::map<int, CacheData> caches;

	void startReading(CacheData& data)
	{
		//logger("ReadLoop::startReading(%d)\n", data.stream.native_handle());
		using std::placeholders::_1;
		using std::placeholders::_2;
		data.stream.async_read_some(
				boost::asio::buffer<char, bufferSize>(data.buffer),
				std::bind(&BasicReadLoop::readFinished, this, std::ref(data),
					_1, _2));
	}

	void readFinished(CacheData& data, boost::system::error_code errorCode,
			std::size_t bytesTransferred)
	{
		//logger("ReadLoop::readFinished(fd=%d, error=%s, bytes=%lu)\n",
				//data.stream.native_handle(), errorCode.message().c_str(),
				//bytesTransferred);

		if (!errorCode) {
			if (bytesTransferred > 0) {
				data.cache.write(data.buffer, bytesTransferred);
				startReading(data);
				return;
			}
		} else {
		}

		data.cache.finish();
		int key = data.stream.native_handle();
		if (data.stream.is_open()) {
			boost::system::error_code errorCode;
			data.stream.close(errorCode);
			// ignore the error
		}

		remove(key);
	}
};

}

#endif /* SRC_CACHE_BASICREADLOOP_HPP */

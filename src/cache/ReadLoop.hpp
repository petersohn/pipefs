#ifndef SRC_CACHE_READLOOP_HPP
#define SRC_CACHE_READLOOP_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <map>
#include <memory>
#include <thread>

namespace pipefs {

class Cache;

class ReadLoop {
public:
	void start();
	void stop();
	void add(int fd, Cache& cache);
	void remove(int fd);
private:
	constexpr static std::size_t bufferSize = 65536;

	boost::asio::io_service ioService;
	std::unique_ptr<std::thread> workingThread;
	std::unique_ptr<boost::asio::io_service::work> work;

	struct CacheData {
		Cache& cache;
		boost::asio::posix::stream_descriptor stream;
		char buffer[bufferSize];

		CacheData(CacheData&&) = default;
		CacheData& operator=(CacheData&&) = default;
	};

	std::map<int, CacheData> caches;

	void startReading(CacheData& data);
	void readFinished(CacheData& data, boost::system::error_code errorCode,
			std::size_t bytesTransferred);
};

}

#endif /* SRC_CACHE_READLOOP_HPP */

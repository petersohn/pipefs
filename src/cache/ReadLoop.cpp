#include "ReadLoop.hpp"
#include "Cache.hpp"
#include "log.h"

#include <functional>

namespace pipefs {

void ReadLoop::start()
{
	if (workingThread) {
		stop();
	}

	log_msg("Starting working thread.\n");

	work = std::make_unique<boost::asio::io_service::work>(ioService);
	workingThread = std::make_unique<std::thread>(
			[this]() { ioService.run(); });
}

void ReadLoop::stop()
{
	if (workingThread) {
		log_msg("Stopping working thread.\n");
		work.reset();

		for (auto& value: caches) {
			value.second.stream.cancel();
		}

		workingThread->join();
		workingThread.reset();
		log_msg("Working thread stopped.\n");
	}
}


void ReadLoop::startReading(CacheData& data)
{
	using std::placeholders::_1;
	using std::placeholders::_2;
	data.stream.async_read_some(
			boost::asio::buffer<char, bufferSize>(data.buffer),
			std::bind(&ReadLoop::readFinished, this, std::ref(data), _1, _2));
}

void ReadLoop::add(int fd, Cache& cache)
{
	auto emplaceResult = caches.emplace(fd,
			CacheData{cache, {ioService, fd}, ""});

	if (emplaceResult.second) {
		auto& data = emplaceResult.first->second;
		startReading(data);
	}
}

void ReadLoop::remove(int fd)
{
	auto it = caches.find(fd);
	if (it != caches.end()) {
		auto& data = it->second;
		data.stream.cancel();
		caches.erase(it);
	}
}

void ReadLoop::readFinished(CacheData& data,
		boost::system::error_code errorCode,
		std::size_t bytesTransferred)
{
	if (!errorCode) {
		log_msg("Read from socket %d: %d bytes",
				data.stream.native_handle(), bytesTransferred);

		if (bytesTransferred > 0) {
			data.cache.write(data.buffer, bytesTransferred);
			startReading(data);
		}
	} else {
		log_msg("Error reading from socket %d: %s\n",
				data.stream.native_handle(), errorCode.message().c_str());
	}
}


}



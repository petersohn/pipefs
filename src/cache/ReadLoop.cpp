#include "ReadLoop.hpp"
#include "Cache.hpp"
#include "log.h"

#include <functional>

namespace pipefs {

void ReadLoop::cancel()
{
	for (auto& value: caches) {
		value.second.stream.cancel();
	}
}


void ReadLoop::startReading(CacheData& data)
{
	//log_msg("ReadLoop::startReading(%d)\n", data.stream.native_handle());
	using std::placeholders::_1;
	using std::placeholders::_2;
	data.stream.async_read_some(
			boost::asio::buffer<char, bufferSize>(data.buffer),
			std::bind(&ReadLoop::readFinished, this, std::ref(data), _1, _2));
}

void ReadLoop::add(ReadStarter readStarter, Cache& cache)
{
	int fd = readStarter();
	log_msg("ReadLoop::add(%d)\n", fd);
	auto emplaceResult = caches.emplace(fd,
			CacheData{cache, {ioService, fd}, ""});

	if (emplaceResult.second) {
		auto& data = emplaceResult.first->second;
		startReading(data);
	}
}

void ReadLoop::remove(int fd)
{
	log_msg("ReadLoop::remove(%d)\n", fd);
	auto it = caches.find(fd);
	if (it != caches.end()) {
		auto& data = it->second;
		boost::system::error_code errorCode;
		data.stream.cancel(errorCode);
		// ignore the error

		caches.erase(it);
	}
}

void ReadLoop::readFinished(CacheData& data,
		boost::system::error_code errorCode,
		std::size_t bytesTransferred)
{
	//log_msg("ReadLoop::readFinished(fd=%d, error=%s, bytes=%lu)\n",
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


}



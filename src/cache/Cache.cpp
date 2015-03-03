#include "Cache.hpp"
#include <cstring>

namespace pipefs {

void Cache::write(const void* buf, std::size_t length)
{
	std::unique_lock<std::mutex> lock{mutex};
	std::size_t position = data.size();
	data.resize(position + length);
	std::memmove(&data[position], buf, length);
	readWaiter.notify_all();
}

int Cache::read(void* buf, std::size_t length, std::size_t position) const
{
	std::unique_lock<std::mutex> lock{mutex};

	while (!finished && position >= data.size()) {
		readWaiter.wait(lock);
	}

	if (finished && position >= data.size()) {
		return 0;
	}

	std::size_t result = std::min(length, data.size() - position);
	std::memmove(buf, &data[position], result);
	return result;
}

void Cache::finish()
{
	std::unique_lock<std::mutex> lock{mutex};
	finished = true;
	readWaiter.notify_all();
}

bool Cache::isFinished() const
{
	std::unique_lock<std::mutex> lock{mutex};
	return finished;
}

std::size_t Cache::getSize() const
{
	std::unique_lock<std::mutex> lock{mutex};
	return data.size();
}

}


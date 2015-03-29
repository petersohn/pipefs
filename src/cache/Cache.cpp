#include "Cache.hpp"
#include <cstring>
#include <log.h>

namespace pipefs {

void Cache::write(const void* buf, std::size_t length)
{
	//log_msg("Cache::write(this=%p, length=%lu); size = %lu\n",
			//this, length, data.size());

	std::unique_lock<std::mutex> lock{mutex};
	std::size_t position = data.size();
	data.resize(position + length);
	std::memmove(&data[position], buf, length);
	readWaiter.notify_all();
}

int Cache::read(void* buf, std::size_t length, std::size_t position) const
{
	std::unique_lock<std::mutex> lock{mutex};
	//log_msg("Cache::read(this=%p, position=%lu, length=%lu); size = %lu\n",
			//this, position, length, data.size());

	while (!finished && position >= data.size()) {
		//log_msg("  blocking...");
		readWaiter.wait(lock);
		//log_msg("  blocking finished");
	}

	if (finished && position >= data.size()) {
		return 0;
	}

	std::size_t result = std::min(length, data.size() - position);
	std::memmove(buf, &data[position], result);
	log_msg("  Result = %d\n", result);
	return result;
}

void Cache::finish()
{
	log_msg("Cache::finish(this=%p)\n", this);
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


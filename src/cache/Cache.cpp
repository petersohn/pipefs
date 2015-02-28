#include "Cache.hpp"
#include <cstring>

namespace pipefs {

void Cache::write(const void* buf, std::size_t length)
{
	std::size_t position = data.size();
	data.resize(position + length);
	std::memmove(&data[position], buf, length);
}

int Cache::read(void* buf, std::size_t length, std::size_t position) const
{
	if (position >= data.size()) {
		return 0;
	}

	std::size_t result = std::min(length, data.size() - position);
	std::memmove(buf, &data[position], result);
	return result;
}

}


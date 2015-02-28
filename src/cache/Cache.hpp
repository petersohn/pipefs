#ifndef SRC_CACHE_CACHE_HPP
#define SRC_CACHE_CACHE_HPP

#include <vector>

namespace pipefs {

class Cache {
public:
	void write(const void* buf, std::size_t length);
	int read(void* buf, std::size_t length, std::size_t position) const;
private:
	std::vector<char> data;
};

}

#endif /* SRC_CACHE_CACHE_HPP */

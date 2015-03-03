#ifndef SRC_CACHE_TIMEUTIL_HPP
#define SRC_CACHE_TIMEUTIL_HPP

#include <chrono>

namespace pipefs {

auto now() {
	return std::chrono::steady_clock::now();
}

};


#endif /* SRC_CACHE_TIMEUTIL_HPP */

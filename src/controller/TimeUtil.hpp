#ifndef SRC_CACHE_TIMEUTIL_HPP
#define SRC_CACHE_TIMEUTIL_HPP

#include <chrono>

namespace pipefs {

struct TimeUtil {
    using time_point = std::chrono::steady_clock::time_point;

    time_point now() {
        return std::chrono::steady_clock::now();
    }
};

}


#endif /* SRC_CACHE_TIMEUTIL_HPP */

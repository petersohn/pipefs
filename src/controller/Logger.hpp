#ifndef SRC_CACHE_LOGGER_HPP
#define SRC_CACHE_LOGGER_HPP

#include "log.h"

#include <utility>

namespace pipefs {

struct Logger {
    template <typename... Args>
    void operator()(Args&&... args)
    {
        log_msg(std::forward<Args>(args)...);
    }
};

}

#endif /* SRC_CACHE_LOGGER_HPP */

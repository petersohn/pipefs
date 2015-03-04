#ifndef SRC_CACHE_CACHES_HPP
#define SRC_CACHE_CACHES_HPP

#include "BasicCaches.hpp"
#include "Cache.hpp"
#include "Logger.hpp"
#include "TimeUtil.hpp"

namespace pipefs {

using Caches = BasicCaches<Cache, Logger, TimeUtil>;

}


#endif /* SRC_CACHE_CACHES_HPP */

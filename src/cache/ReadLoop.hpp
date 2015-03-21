#ifndef SRC_CACHE_READLOOP_HPP
#define SRC_CACHE_READLOOP_HPP

#include <boost/asio/posix/stream_descriptor.hpp>

#include "BasicReadLoop.hpp"
#include "Cache.hpp"
#include "Logger.hpp"

namespace pipefs {

using ReadLoop = BasicReadLoop<boost::asio::posix::stream_descriptor, Cache,
		  Logger>;

}

#endif /* SRC_CACHE_READLOOP_HPP */

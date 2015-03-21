#ifndef SRC_CACHE_READLOOP_HPP
#define SRC_CACHE_READLOOP_HPP

#include <boost/asio/posix/stream_descriptor.hpp>

#include "BasicReadLoop.hpp"

namespace pipefs {

using ReadLoop = BasicReadLoop<boost::asio::posix::stream_descriptor>;

}

#endif /* SRC_CACHE_READLOOP_HPP */

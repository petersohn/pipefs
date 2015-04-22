#ifndef SRC_CACHE_SYSTEMERROR_HPP
#define SRC_CACHE_SYSTEMERROR_HPP

#include <boost/system/system_error.hpp>
#include <errno.h>

namespace pipefs {

inline
void throwSystemError(int errorCode)
{
    using namespace boost::system;
    throw system_error{errorCode, system_category()};
}

inline
void throwError()
{
    throwSystemError(errno);
}

template <typename Function, typename... Args>
int checkedSystemCall(Function function, Args... args) {
    int result = function(std::forward<Args>(args)...);
    if (result < 0) {
        throwError();
    }
    return result;
}

};


#endif /* SRC_CACHE_SYSTEMERROR_HPP */


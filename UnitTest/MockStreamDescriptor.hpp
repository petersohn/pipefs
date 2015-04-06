#ifndef UNITTEST_MOCKSTREAMDESCRIPTOR_HPP
#define UNITTEST_MOCKSTREAMDESCRIPTOR_HPP

#include <turtle/mock.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <set>
#include <functional>

#define METHOD_WITH_ERROR(name) \
    void name() \
    { \
        boost::system::error_code errorCode; \
        name(errorCode); \
        if (errorCode) { \
            throw boost::system::system_error{errorCode}; \
        } \
    }

namespace pipefs {

struct MockStreamDescriptor {
    using native_handle_type = int;

    MockStreamDescriptor(boost::asio::io_service& ioService):
            ioService(ioService) {}

    MOCK_METHOD(cancel, 1, void(boost::system::error_code&));
    METHOD_WITH_ERROR(cancel)

    MOCK_METHOD(close, 1, void(boost::system::error_code&));
    METHOD_WITH_ERROR(close)

    using AsyncReadResult = std::pair<boost::system::error_code, std::size_t>;
    MOCK_METHOD(doAsyncRead, 0, AsyncReadResult());
    template <typename Buffer, typename Callback>
    void async_read_some(const Buffer&, const Callback& callback)
    {
        auto result = doAsyncRead();
        ioService.post(std::bind(callback, result.first, result.second));
    }

    MOCK_METHOD(native_handle, 0, native_handle_type());
    MOCK_METHOD(is_open, 0, bool());

    boost::asio::io_service& ioService;
};

}

#undef METHOD_WITH_ERROR

#endif /* UNITTEST_MOCKSTREAMDESCRIPTOR_HPP */

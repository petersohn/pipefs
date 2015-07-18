#ifndef SRC_CACHE_SIGNALHANDLER_HPP
#define SRC_CACHE_SIGNALHANDLER_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

#include <memory>

namespace pipefs {

class SignalHandler: public std::enable_shared_from_this<SignalHandler> {
public:
    SignalHandler(boost::asio::io_service& ioService);
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;

    void start();
    void cancel();
private:
    boost::asio::io_service& ioService;
    boost::asio::signal_set signalSet{ioService, SIGCHLD};

    void wait();
};

}

#endif /* SRC_CACHE_SIGNALHANDLER_HPP */

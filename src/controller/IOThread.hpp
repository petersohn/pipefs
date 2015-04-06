#ifndef SRC_CACHE_IOTHREAD_HPP
#define SRC_CACHE_IOTHREAD_HPP

#include <boost/asio/io_service.hpp>
#include <memory>
#include <thread>

namespace pipefs {

class IOThread {
public:
    IOThread() = default;
    IOThread(const IOThread&) = delete;
    IOThread& operator=(const IOThread&) = delete;
    IOThread(IOThread&&) = delete;
    IOThread& operator=(IOThread&&) = delete;

    void start();
    void stop();
    boost::asio::io_service& getIoService() { return ioService; }
private:
    boost::asio::io_service ioService;
    std::unique_ptr<std::thread> workingThread;
    std::unique_ptr<boost::asio::io_service::work> work;
};

}


#endif /* SRC_CACHE_IOTHREAD_HPP */

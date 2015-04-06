#include "IOThread.hpp"
#include "log.h"

namespace pipefs {

void IOThread::start()
{
    if (workingThread) {
        stop();
    }

    log_msg("Starting working thread.\n");

    work = std::make_unique<boost::asio::io_service::work>(ioService);
    workingThread = std::make_unique<std::thread>(
            [this]() { ioService.run(); });
}

void IOThread::stop()
{
    if (workingThread) {
        log_msg("Stopping working thread.\n");
        work.reset();

        workingThread->join();
        workingThread.reset();
        log_msg("Working thread stopped.\n");
    }
}

}

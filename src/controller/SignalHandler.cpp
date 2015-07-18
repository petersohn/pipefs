#include "SignalHandler.hpp"
#include "log.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

namespace pipefs {

SignalHandler::SignalHandler(boost::asio::io_service& ioService): ioService(ioService)
{
}

void SignalHandler::start()
{
    wait();
}

void SignalHandler::cancel()
{
    signalSet.cancel();
}

void SignalHandler::wait()
{
    auto self = shared_from_this();
    signalSet.async_wait(
            [this, self](boost::system::error_code errorCode, int /*signal*/) {
                if (!errorCode) {
                    log_msg("Got SIGCHLD signal");
                    int result = ::wait(nullptr);
                    log_msg("  wait result = %d; error = \n", result,
                            strerror(errno));
                    wait();
                } else {
                    std::string errorMsg = errorCode.message();
                    log_msg("Signal handler exiting. Reason: %s\n",
                            errorMsg.c_str());
                }
            });
}

}

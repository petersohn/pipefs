#include "SignalHandler.hpp"

#include <sys/types.h>
#include <sys/wait.h>

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
    signalSet.async_wait([this](boost::system::error_code errorCode,
                int /*signal*/)
        {
            if (!errorCode) {
                ::wait(NULL);
                wait();
            }
        });
}

}

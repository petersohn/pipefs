#include "SpawnCommand.hpp"
#include "FileData.hpp"
#include "SystemError.hpp"

#include <util/Finally.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

namespace pipefs {

int spawnCommand(const char* command, int fd, int flags, FileData* fileData)
{
    int pipefd[2];
    checkedSystemCall(&pipe, pipefd);
    auto pipeClose = util::finally([pipefd]() { close(pipefd[1]); });
    int pid = 0;

    try {
        checkedSystemCall(&fcntl, pipefd[0], F_SETFL, flags);
        pid = checkedSystemCall(&fork);

        if (pid == 0) { // child
            close(pipefd[0]);
            if (dup2(fd, 0) < 0) {
                perror("dup2(fd, 0)");
                _exit(1);
            }
            if (dup2(pipefd[1], 1) < 0) {
                perror("dup2(pipefd[1], 1)");
                _exit(1);
            }
            close(fd);
            close(pipefd[1]);

            struct sigaction signal_action;
            memset(&signal_action, 0, sizeof(signal_action));
            signal_action.sa_handler = SIG_DFL;
            for (int signal = 1; signal < _NSIG; ++signal) {
                sigaction(signal, &signal_action, nullptr);
            }

            execl("/bin/sh", "/bin/sh", "-c", command, static_cast<char*>(nullptr));
            // at this point, execl() is unsuccessful
            _exit(1);
        }
    } catch (...) {
        close(pipefd[0]);
        throw;
    }

    // parent
    if (fileData) {
        fileData->fd = pipefd[0];
        fileData->pid = pid;
        fileData->currentOffset = 0;
    }

    return pipefd[0];
}

};



#include "signal_handler.h"
#include "params.h"

#include <signal.h>
#include <sys/wait.h>
#include <string.h>

void signal_handler_initialize()
{
    sigset_t sigset;
    sigemptyset(&sigset);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    action.sa_mask = sigset;
    action.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT;

    sigaction(SIGCHLD, &action, NULL);
}


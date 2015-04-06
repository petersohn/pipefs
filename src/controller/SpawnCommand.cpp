#include "SpawnCommand.hpp"
#include "FileData.hpp"
#include "SystemError.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

namespace pipefs {

void spawnCommand(const char* command, const char* inputFile, int flags,
		FileData& fileData)
{
	int fd = open(inputFile, O_RDONLY);
	if (fd < 0) {
		throwError();
	}

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		close(fd);
		throwError();
	}

	if (fcntl(pipefd[0], F_SETFL, flags) < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		close(fd);
		throwError();
	}

	int pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		close(fd);
		throwError();
	}

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

	// parent
	close(pipefd[1]);

	fileData.originalFd = fd;
	fileData.fd = pipefd[0];
	fileData.pid = pid;
	fileData.currentOffset = 0;
}

};



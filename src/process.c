#include "process.h"
#include "params.h"
#include "data.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

int spawn_command(const char* command, const char* input_file, int flags,
		struct pipefs_filedata* filedata)
{
	int fd = open(input_file, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		close(fd);
		return -1;
	}

	if (fcntl(pipefd[0], F_SETFL, flags) < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		close(fd);
		return -1;
	}

	int pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		close(fd);
		return -1;
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
			sigaction(signal, &signal_action, NULL);
		}

		execl("/bin/sh", "/bin/sh", "-c", command, (char*)NULL);
		// at this point, execl() is unsuccessful
		_exit(1);
	}

	// parent
	close(pipefd[1]);

	if (filedata) {
		filedata->original_fd = fd;
		filedata->fd = pipefd[0];
		filedata->pid = pid;
		filedata->current_offset = 0;
	} else {
		close(fd);
	}
	return pipefd[0];
}


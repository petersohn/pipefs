#include "process.h"
#include "params.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int spawn_command(const char* command, const char* input_file)
{
	int fd = open(input_file, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	int pipefd[2];
	int result = pipe(pipefd);
	if (result < 0) {
		return -1;
	}

	int pid = fork();
	if (pid < 0) {
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
		execl("/bin/sh", "/bin/sh", "-c", command, (char*)NULL);
		// at this point, execl() is unsuccessful
		_exit(1);
	}

	// parent
	close(fd);
	close(pipefd[1]);
	return pipefd[0];
}


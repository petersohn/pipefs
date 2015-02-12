#include "process.h"
#include "params.h"

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <sys/wait.h>
#include <unistd.h>


#define CHECKED_SYSCALL(syscall, result) do { \
	(result) = (syscall); \
	if ((result) == -1 && errno != EINTR) { \
		BOOST_FAIL(#result " failed: " << strerror(errno)); \
	} \
} while ((result) == -1)


BOOST_AUTO_TEST_SUITE(ProcessTest)

BOOST_AUTO_TEST_CASE(successful_process)
{
	int fd = 0;
	CHECKED_SYSCALL(spawn_command("true", "/dev/null"), fd);
}

BOOST_AUTO_TEST_CASE(process_with_output)
{
	std::string text = "something";
	std::string command = "echo -n " + text;
	int fd = 0;
	CHECKED_SYSCALL(spawn_command(command.c_str(), "/dev/null"), fd);

	char result[20];
	ssize_t readBytes = 0;
	CHECKED_SYSCALL(read(fd, result, 20), readBytes);
	result[readBytes] = 0;
	BOOST_CHECK_EQUAL(readBytes, text.size());
	BOOST_CHECK_EQUAL(result, text);
	readBytes = read(fd, result, 20);
	CHECKED_SYSCALL(read(fd, result, 20), readBytes);
	BOOST_CHECK_EQUAL(readBytes, 0);
}

BOOST_AUTO_TEST_SUITE_END()



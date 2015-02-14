#include "process.h"
#include "params.h"
#include "data.h"

#include "System.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>



BOOST_AUTO_TEST_SUITE(ProcessTest)

BOOST_AUTO_TEST_CASE(successful_process)
{
	struct pipefs_filedata data;
	int fd = 0;
	CHECKED_SYSCALL(spawn_command("true", "/dev/null", O_RDONLY, &data), fd);
	BOOST_CHECK_EQUAL(data.fd, fd);
}

BOOST_AUTO_TEST_CASE(process_with_output)
{
	std::string text = "something";
	std::string command = "echo -n " + text;
	int fd = 0;
	CHECKED_SYSCALL(spawn_command(command.c_str(), "/dev/null", O_RDONLY, NULL), fd);

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

struct FileFixture {
	std::string filename = "input.txt";
	std::ofstream file{filename, std::ios::out | std::ios::trunc};
	~FileFixture()
	{
		file.close();
		unlink(filename.c_str());
	}
};

BOOST_FIXTURE_TEST_CASE(process_with_input, FileFixture)
{
	std::string text = "something";
	file << text;
	file.close();

	int fd = 0;
	CHECKED_SYSCALL(spawn_command("cat", filename.c_str(), O_RDONLY, NULL), fd);

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



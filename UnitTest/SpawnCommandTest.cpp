#include "controller/SpawnCommand.hpp"
#include "controller/FileData.hpp"
#include "System.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

using namespace pipefs;

struct ProcessTestFixture {
    std::string filename = "input.txt";
    std::ofstream file{filename, std::ios::out | std::ios::trunc};
    int readFd;

    ProcessTestFixture()
    {
        CHECKED_SYSCALL(open(filename.c_str(), O_RDONLY), readFd);
    }

    ~ProcessTestFixture()
    {
        close(readFd);
        file.close();
        unlink(filename.c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(ProcessTest, ProcessTestFixture)

BOOST_AUTO_TEST_CASE(successful_process)
{
    FileData data;
    int fd = spawnCommand("true", readFd, O_RDONLY, &data);
    BOOST_CHECK_EQUAL(fd, data.fd);
}

BOOST_AUTO_TEST_CASE(successful_process_with_no_data)
{
    spawnCommand("true", readFd, O_RDONLY, nullptr);
}

BOOST_AUTO_TEST_CASE(process_with_output)
{
    std::string text = "something";
    std::string command = "echo -n " + text;
    int fd = spawnCommand(command.c_str(), readFd, O_RDONLY, nullptr);

    char result[20];
    ssize_t readBytes = 0;
    CHECKED_SYSCALL(read(fd, result, 20), readBytes);
    result[readBytes] = 0;
    BOOST_CHECK_EQUAL(readBytes, text.size());
    BOOST_CHECK_EQUAL(result, text);
    CHECKED_SYSCALL(read(fd, result, 20), readBytes);
    BOOST_CHECK_EQUAL(readBytes, 0);
}

BOOST_AUTO_TEST_CASE(FileFixture)
{
    std::string text = "something";
    file << text;
    file.close();

    int fd = spawnCommand("cat", readFd, O_RDONLY, nullptr);

    char result[20];
    ssize_t readBytes = 0;
    CHECKED_SYSCALL(read(fd, result, 20), readBytes);
    result[readBytes] = 0;
    BOOST_CHECK_EQUAL(readBytes, text.size());
    BOOST_CHECK_EQUAL(result, text);
    CHECKED_SYSCALL(read(fd, result, 20), readBytes);
    BOOST_CHECK_EQUAL(readBytes, 0);
}

BOOST_AUTO_TEST_SUITE_END()



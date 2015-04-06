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

BOOST_AUTO_TEST_SUITE(ProcessTest)

BOOST_AUTO_TEST_CASE(successful_process)
{
    FileData data;
    spawnCommand("true", "/dev/null", O_RDONLY, data);
}

BOOST_AUTO_TEST_CASE(process_with_output)
{
    std::string text = "something";
    std::string command = "echo -n " + text;
    FileData data;
    spawnCommand(command.c_str(), "/dev/null", O_RDONLY, data);

    char result[20];
    ssize_t readBytes = 0;
    CHECKED_SYSCALL(read(data.fd, result, 20), readBytes);
    result[readBytes] = 0;
    BOOST_CHECK_EQUAL(readBytes, text.size());
    BOOST_CHECK_EQUAL(result, text);
    CHECKED_SYSCALL(read(data.fd, result, 20), readBytes);
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

    FileData data;
    spawnCommand("cat", filename.c_str(), O_RDONLY, data);

    char result[20];
    ssize_t readBytes = 0;
    CHECKED_SYSCALL(read(data.fd, result, 20), readBytes);
    result[readBytes] = 0;
    BOOST_CHECK_EQUAL(readBytes, text.size());
    BOOST_CHECK_EQUAL(result, text);
    CHECKED_SYSCALL(read(data.fd, result, 20), readBytes);
    BOOST_CHECK_EQUAL(readBytes, 0);
}

BOOST_AUTO_TEST_SUITE_END()



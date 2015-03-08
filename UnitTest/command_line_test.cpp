#include "command_line.h"
#include "data.h"
#include "System.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

#define _CHECK_ARRAY_ELEMENT(r, array, i, elem) \
	BOOST_CHECK_EQUAL(array[i], elem);
#define CHECK_ARRAY(array, elements) do { \
	BOOST_PP_SEQ_FOR_EACH_I(_CHECK_ARRAY_ELEMENT, array, elements) \
} while(false)

#define _STRINGIZE_SEQ_ELEM(r, data, elem) (std::string{elem})
#define STRINGIZE_SEQ(seq) BOOST_PP_SEQ_FOR_EACH(_STRINGIZE_SEQ_ELEM, _, seq)

#define CHECK_STRING(chararray, string) do { \
	BOOST_CHECK(chararray); \
	if (chararray) { \
		BOOST_CHECK_EQUAL(chararray, string); \
	} \
} while (false)

struct ParseSizeTestFixture {
	std::size_t testParseSize(std::size_t size, char unit)
	{
		std::string inputString = boost::lexical_cast<std::string>(size) + unit;
		return parse_size(inputString.c_str());
	}
};

BOOST_FIXTURE_TEST_SUITE(ParseSizeTest, ParseSizeTestFixture)

BOOST_AUTO_TEST_CASE(no_unit)
{
	std::size_t value = 2354;
	std::string inputString = boost::lexical_cast<std::string>(value);
	std::size_t result = parse_size(inputString.c_str());
	BOOST_CHECK_EQUAL(result, value);
}

BOOST_AUTO_TEST_CASE(unit_b)
{
	std::size_t value = 3457;
	std::size_t result = testParseSize(value, 'b');
	BOOST_CHECK_EQUAL(result, value);
}

BOOST_AUTO_TEST_CASE(unit_B)
{
	std::size_t value = 378314;
	std::size_t result = testParseSize(value, 'B');
	BOOST_CHECK_EQUAL(result, value);
}

BOOST_AUTO_TEST_CASE(unit_k)
{
	std::size_t value = 8222;
	std::size_t result = testParseSize(value, 'k');
	BOOST_CHECK_EQUAL(result, value * 1024);
}

BOOST_AUTO_TEST_CASE(unit_K)
{
	std::size_t value = 6210;
	std::size_t result = testParseSize(value, 'K');
	BOOST_CHECK_EQUAL(result, value * 1000);
}

BOOST_AUTO_TEST_CASE(unit_m)
{
	std::size_t value = 1265;
	std::size_t result = testParseSize(value, 'm');
	BOOST_CHECK_EQUAL(result, value * 1024 * 1024);
}

BOOST_AUTO_TEST_CASE(unit_M)
{
	std::size_t value = 384;
	std::size_t result = testParseSize(value, 'M');
	BOOST_CHECK_EQUAL(result, value * 1000 * 1000);
}

BOOST_AUTO_TEST_CASE(unit_g)
{
	std::size_t value = 1;
	std::size_t result = testParseSize(value, 'g');
	BOOST_CHECK_EQUAL(result, value * 1024 * 1024 * 1024);
}

BOOST_AUTO_TEST_CASE(unit_G)
{
	std::size_t value = 2;
	std::size_t result = testParseSize(value, 'G');
	BOOST_CHECK_EQUAL(result, value * 1000 * 1000 * 1000);
}

BOOST_AUTO_TEST_SUITE_END()

struct CommandLineFixture {
	char** result = nullptr;
	int resultLength;
	struct pipefs_data data;

	CommandLineFixture()
	{
		memset(&data, 0, sizeof(data));
	}

	~CommandLineFixture()
	{
		free(result);
	}
};

BOOST_FIXTURE_TEST_SUITE(CommandLineTest, CommandLineFixture)

BOOST_AUTO_TEST_CASE(only_fuse_params)
{
	const char* args[] = {"pipefs", "first", "second", "third"};
	result = parse_arguments(4, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 4);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")("first")("second")("third")));
}

BOOST_AUTO_TEST_CASE(parse_command)
{
	std::string value = "some command";
	const char* args[] = {"pipefs", "--command", value.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	CHECK_STRING(data.command, value);
}

BOOST_AUTO_TEST_CASE(parse_source_suffix)
{
	std::string value = "some suffix";
	const char* args[] = {"pipefs", "--source-suffix", value.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	CHECK_STRING(data.source_suffix, value);
}

BOOST_AUTO_TEST_CASE(parse_target_suffix)
{
	std::string value = "some suffix";
	const char* args[] = {"pipefs", "--target-suffix", value.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	CHECK_STRING(data.target_suffix, value);
}

BOOST_AUTO_TEST_CASE(parse_root_dir)
{
	std::string value = "some dir";
	const char* args[] = {"pipefs", "--root-dir", value.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	CHECK_STRING(data.rootdir, value);
}

BOOST_AUTO_TEST_CASE(parse_log_file)
{
	std::string value = "log.txt";
	const char* args[] = {"pipefs", "--log-file", value.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	int unlinkResult;
	CHECKED_SYSCALL(unlink(value.c_str()), unlinkResult);
}

BOOST_AUTO_TEST_CASE(parse_seekable)
{
	const char* args[] = {"pipefs", "--seekable"};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	BOOST_CHECK_EQUAL(data.seekable, 1);
	BOOST_CHECK_EQUAL(data.cache, 0);
}

BOOST_AUTO_TEST_CASE(parse_cache)
{
	const char* args[] = {"pipefs", "--cache"};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	BOOST_CHECK_EQUAL(data.seekable, 1);
	BOOST_CHECK_EQUAL(data.cache, 1);
}

BOOST_AUTO_TEST_CASE(parse_cache_limit)
{
	std::string valueString = "463467";
	std::size_t value = boost::lexical_cast<std::size_t>(valueString);
	const char* args[] = {"pipefs", "--cache-limit", valueString.c_str()};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	BOOST_CHECK_EQUAL(data.cache_limit, value);
}

BOOST_AUTO_TEST_CASE(parse_all_values)
{
	std::string command = "command";
	std::string sourceSuffix = "source suffix";
	std::string targetSuffix = "target suffix";
	std::string rootDir = "rootDir";
	const char* args[] = {
			"pipefs",
			"--root-dir", rootDir.c_str(),
			"--command", command.c_str(),
			"--source-suffix", sourceSuffix.c_str(),
			"--target-suffix", targetSuffix.c_str(),
		};
	result = parse_arguments(9, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 1);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("pipefs")));
	CHECK_STRING(data.rootdir, rootDir);
	CHECK_STRING(data.command, command);
	CHECK_STRING(data.source_suffix, sourceSuffix);
	CHECK_STRING(data.target_suffix, targetSuffix);
}

BOOST_AUTO_TEST_CASE(parse_all_values_plus_fuse_options)
{
	std::string command = "command";
	std::string sourceSuffix = "source suffix";
	std::string targetSuffix = "target suffix";
	std::string rootDir = "rootDir";
	std::string arg1 = "--arg1";
	std::string arg2 = "arg2";
	std::string arg3 = "-arg3";
	std::string arg4 = "--arg4";
	const char* args[] = {
			"pipefs",
			"--root-dir", rootDir.c_str(),
			"--command", command.c_str(),
			"--source-suffix", sourceSuffix.c_str(),
			"--target-suffix", targetSuffix.c_str(),
			"--", arg1.c_str(), arg2.c_str(), arg3.c_str(), arg4.c_str(),
		};
	result = parse_arguments(14, const_cast<char**>(args), &data, &resultLength);
	CHECK_STRING(data.rootdir, rootDir);
	CHECK_STRING(data.command, command);
	CHECK_STRING(data.source_suffix, sourceSuffix);
	CHECK_STRING(data.target_suffix, targetSuffix);
	BOOST_REQUIRE_EQUAL(resultLength, 5);
	CHECK_ARRAY(result, (std::string{"pipefs"})(arg1)(arg2)(arg3)(arg4));
}

BOOST_AUTO_TEST_SUITE_END()

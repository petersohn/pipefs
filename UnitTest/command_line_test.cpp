#include "command_line.h"
#include "data.h"
#include "System.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
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


struct CommandLineFixture {
	char** result = nullptr;
	int resultLength;
	struct pipefs_data data;

	~CommandLineFixture()
	{
		free(result);
	}
};

BOOST_FIXTURE_TEST_SUITE(CommandLineTest, CommandLineFixture)

BOOST_AUTO_TEST_CASE(only_fuse_params)
{
	const char* args[] = {"first", "second", "third"};
	result = parse_arguments(3, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 3);
	CHECK_ARRAY(result, STRINGIZE_SEQ(("first")("second")("third")));
}

BOOST_AUTO_TEST_CASE(parse_command)
{
	std::string value = "some command";
	const char* args[] = {"--command", value.c_str()};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
	CHECK_STRING(data.command, value);
}

BOOST_AUTO_TEST_CASE(parse_source_suffix)
{
	std::string value = "some suffix";
	const char* args[] = {"--source-suffix", value.c_str()};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
	CHECK_STRING(data.source_suffix, value);
}

BOOST_AUTO_TEST_CASE(parse_target_suffix)
{
	std::string value = "some suffix";
	const char* args[] = {"--target-suffix", value.c_str()};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
	CHECK_STRING(data.target_suffix, value);
}

BOOST_AUTO_TEST_CASE(parse_root_dir)
{
	std::string value = "some dir";
	const char* args[] = {"--root-dir", value.c_str()};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
	CHECK_STRING(data.rootdir, value);
}

BOOST_AUTO_TEST_CASE(parse_log_file)
{
	std::string value = "log.txt";
	const char* args[] = {"--log-file", value.c_str()};
	result = parse_arguments(2, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
	int unlinkResult;
	CHECKED_SYSCALL(unlink(value.c_str()), unlinkResult);
}

BOOST_AUTO_TEST_CASE(parse_all_values)
{
	std::string command = "command";
	std::string sourceSuffix = "source suffix";
	std::string targetSuffix = "target suffix";
	std::string rootDir = "rootDir";
	const char* args[] = {
			"--root-dir", rootDir.c_str(),
			"--command", command.c_str(),
			"--source-suffix", sourceSuffix.c_str(),
			"--target-suffix", targetSuffix.c_str(),
		};
	result = parse_arguments(8, const_cast<char**>(args), &data, &resultLength);
	BOOST_CHECK_EQUAL(resultLength, 0);
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
			arg1.c_str(), arg2.c_str(),
			"--root-dir", rootDir.c_str(),
			"--command", command.c_str(),
			arg3.c_str(),
			"--source-suffix", sourceSuffix.c_str(),
			"--target-suffix", targetSuffix.c_str(),
			arg4.c_str(),
		};
	result = parse_arguments(12, const_cast<char**>(args), &data, &resultLength);
	CHECK_STRING(data.rootdir, rootDir);
	CHECK_STRING(data.command, command);
	CHECK_STRING(data.source_suffix, sourceSuffix);
	CHECK_STRING(data.target_suffix, targetSuffix);
	BOOST_REQUIRE_EQUAL(resultLength, 4);
	CHECK_ARRAY(result, (arg1)(arg2)(arg3)(arg4));
}

BOOST_AUTO_TEST_SUITE_END()

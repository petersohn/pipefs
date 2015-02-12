#include "command_line.h"
#include "data.h"

#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>

#define _CHECK_ARRAY_ELEMENT(r, array, i, elem) \
	BOOST_CHECK_EQUAL(array[i], elem);

#define CHECK_ARRAY(array, elements) do { \
	BOOST_PP_SEQ_FOR_EACH_I(_CHECK_ARRAY_ELEMENT, array, elements) \
} while(false)

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
	CHECK_ARRAY(result, (std::string{"first"})(std::string{"second"})(std::string{"third"}));
}

BOOST_AUTO_TEST_SUITE_END()

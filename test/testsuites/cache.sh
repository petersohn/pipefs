#!/bin/bash

cache_limit=1k

function test_init()
{
	common_init
	executed_filename="$PWD/executed_num"
	rm -f "$executed_filename"
	if [ -z "$command" ]; then
		command="echo line >>$executed_filename && cat"
	fi

	custom_options=(--cache --cache-limit "$cache_limit")
	start_pipefs
}

function test_cleanup()
{
	common_cleanup
}

function get_test_cases()
{
	get_common_test_cases
	echo "testcases/can_seek_in_translated_files.sh"
	echo "testcases/command_is_only_executed_once.sh"
	echo "testcases/command_is_executed_more_times_if_file_is_large.sh"
	echo "testcases/command_is_executed_only_once_if_read_in_parallel.sh"
	echo "testcases/cache_is_not_cleared_when_cache_limit_is_set_to_zero.sh"
}





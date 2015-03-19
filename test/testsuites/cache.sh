#!/bin/bash

function test_init()
{
	common_init
	executed_filename="$PWD/executed_num"
	rm -f "$executed_filename"
	command="echo line >>$executed_filename && cat"
	assert "$binary_name" --cache --cache-limit=1k --log-file "$log_dir/pipefs.log" --source-suffix .1 --target-suffix .2 --command "$(get_command)" --root-dir rootdir mountpoint
	wait_for_fuse
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
}





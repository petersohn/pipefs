#!/bin/bash

function test_init()
{
	common_init
	assert "$binary_name" --seekable --log-file "$log_dir/pipefs.log" --source-suffix .1 --target-suffix .2 --command "$(get_command)" --root-dir rootdir mountpoint
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
}




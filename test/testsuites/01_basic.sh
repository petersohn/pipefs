#!/bin/bash

function test_init()
{
	common_init
	assert "$binary_name" --log-file "$log_dir/pipefs.log" --source-suffix .1 --target-suffix .2 --command cat --root-dir rootdir mountpoint
	sleep 0.1
}

function test_cleanup()
{
	common_cleanup
}

function get_test_cases()
{
	echo "testcases/01_non_suffix_files_remain_unchanged.sh"
}



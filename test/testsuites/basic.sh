#!/bin/bash

function test_init()
{
	common_init
	assert "$binary_name" --log-file "$log_dir/pipefs.log" --source-suffix .1 --target-suffix .2 --command tac --root-dir rootdir mountpoint
	sleep 0.1
}

function test_cleanup()
{
	common_cleanup
}

function get_test_cases()
{
	echo "testcases/can_find_files_in_directories.sh"
	echo "testcases/directory_listing_works_for_non_suffix_files.sh"
	echo "testcases/directory_listing_works_for_suffix_files.sh"
	echo "testcases/can_find_suffix_files_in_directories.sh"
	echo "testcases/non_suffix_files_remain_unchanged.sh"
	echo "testcases/can_write_to_non_suffix_files_on_mountpoint.sh"
	echo "testcases/can_create_non_suffix_files_on_mountpoint.sh"
}



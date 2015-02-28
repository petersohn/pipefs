#!/bin/bash

function test_init()
{
	common_init
	assert "$binary_name" --source-suffix .1 --target-suffix .2 --command tac --root-dir rootdir mountpoint
	wait_for_fuse
}

function test_cleanup()
{
	common_cleanup
}

function get_test_cases()
{
	get_common_test_cases
}




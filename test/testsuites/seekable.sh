#!/bin/bash

function test_init()
{
	common_init
	custom_options=(--seekable)
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
}




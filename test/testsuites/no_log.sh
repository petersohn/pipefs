#!/bin/bash

function test_init()
{
	common_init
	log_options=()
	start_pipefs
}

function test_cleanup()
{
	common_cleanup
}

function get_test_cases()
{
	get_common_test_cases
}




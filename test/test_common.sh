#!/bin/bash

function common_init()
{
	mkdir rootdir
	mkdir mountpoint
}

function run_with_tries()
{
	local tries=$1
	shift
	while [ $tries -gt 0 ] && ! "$@"; do
		(( --tries ))
		sleep 0.01
	done

	[ $tries -gt 0 ]
}

function get_command()
{
	echo "${command:-sed 's/^/->/'}"
}

function get_expected_file_contents()
{
	eval "$(get_command)" <"$1"
}

function wait_for_fuse()
{
	if ! run_with_tries 100 ls mountpoint >/dev/null 2>&1; then
		echo "Failed to mount filesystem."
		exit 1
	fi
}

function common_cleanup()
{
	if ! run_with_tries 100 eval '[ ! -e mountpoint ] || fusermount -u mountpoint && rm -rf mountpoint && [ ! -e mountpoint ]'; then
		echo "Failed to remove mountpoint."
		exit 1
	fi
}

function get_common_test_cases()
{
	echo "testcases/can_find_files_in_directories.sh"
	echo "testcases/directory_listing_works_for_suffix_files.sh"
	echo "testcases/can_find_suffix_files_in_directories.sh"
	echo "testcases/suffix_files_are_translated.sh"
	echo "testcases/translation_works_for_large_files.sh"
	echo "testcases/translation_works_in_parallel.sh"
	echo "testcases/file_descriptors_are_not_leaked.sh"
}


#!/bin/bash

function common_init()
{
	mkdir rootdir
	mkdir mountpoint
	log_options=(--log-file "$log_dir/pipefs.log")
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

	assert [ -f pidfile ]
	local command_pid=$(<pidfile)
	cp pidfile pidfile2
	local cmdline_filename="/proc/$command_pid/cmdline"
	assert [ -f "$cmdline_filename" ]

	local pidfile_command_line=()
	while read -r -d '' arg; do
		pidfile_command_line+=("$arg")
	done < "$cmdline_filename"

	assert [ "${pidfile_command_line[*]}" == "${command_line[*]}" ]
}

function start_pipefs() {
	command_line=("$binary_name" "${custom_options[@]}" ${log_options[@]} --pidfile pidfile --source-suffix .1 --target-suffix .2 --command "$(get_command)" --root-dir rootdir mountpoint)
	assert "${command_line[@]}"
	wait_for_fuse
}

function common_cleanup()
{
	local result=
	if ! run_with_tries 100 eval '[ ! -e mountpoint ] || fusermount -u mountpoint && rm -rf mountpoint && [ ! -e mountpoint ]'; then
		echo "Failed to remove mountpoint."
		result=1
	fi

	local command_pid=$(<pidfile2)
	if ! run_with_tries 100 [ ! -e "/proc/$command_pid" ]; then
		echo "The pipefs process did not die."
		kill -9 "-$command_pid"
		fusermount -u mountpoint
		result=1
	fi

	assert [ ! -e pidfile ]

	return $result
}

function get_common_test_cases()
{
	echo "testcases/can_find_files_in_directories"
	echo "testcases/directory_listing_works_for_suffix_files"
	echo "testcases/can_find_suffix_files_in_directories"
	echo "testcases/suffix_files_are_translated"
	echo "testcases/translation_works_for_large_files"
	echo "testcases/translation_works_in_parallel"
	echo "testcases/translation_works_after_interrupting_a_read"
	echo "testcases/file_descriptors_are_not_leaked"
}

custom_options=()


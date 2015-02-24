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

function wait_for_fuse()
{
	if ! run_with_tries 100 ls mountpoint >/dev/null 2>&1; then
		echo "Failed to mount filesystem."
		exit 1
	fi
}

function common_cleanup()
{
	if ! run_with_tries 100 eval 'fusermount -u mountpoint && rm -rf mountpoint && [ ! -e mountpoint ]'; then
		echo "Failed to remove mountpoint."
		exit 1
	fi
	rm -rf rootdir
}


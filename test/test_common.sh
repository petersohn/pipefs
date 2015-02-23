#!/bin/bash

function common_init()
{
	mkdir rootdir
	mkdir mountpoint
}

function wait_for_fuse()
{
	local tries=100
	while [ $tries -gt 0 ] && ! ls mountpoint >/dev/null 2>&1; do
		(( --tries ))
		sleep 0.01
	done
	if [ $tries -eq 0 ]; then
		echo "Failed to mount filesystem."
		exit 1
	fi
}

function common_cleanup()
{
	fusermount -u mountpoint
	rm -rf rootdir
	rm -rf mountpoint
}


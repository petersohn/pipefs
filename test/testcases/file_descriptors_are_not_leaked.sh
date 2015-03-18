#!/bin/bash

function test_run()
{
	local number_of_files=30
	for ((i=0; i<number_of_files; ++i)); do
		echo "This is file $i" >rootdir/file${i}.1
	done

	ulimit -n 20

	for ((i=0; i<number_of_files; ++i)); do
		cat mountpoint/file${i}.2 >/dev/null
	done
}



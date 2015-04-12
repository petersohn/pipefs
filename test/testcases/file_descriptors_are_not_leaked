#!/bin/bash

ulimit -n 30

function test_run()
{
	local number_of_files=50
	for ((i=0; i<number_of_files; ++i)); do
		echo "This is file $i" >rootdir/file${i}.1
	done

	for ((i=0; i<number_of_files; ++i)); do
		cat mountpoint/file${i}.2 >/dev/null
	done
}



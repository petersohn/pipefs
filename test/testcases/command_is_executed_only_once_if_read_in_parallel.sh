#!/bin/bash

cache_limit=1m

function test_run()
{
    local filename=filename.1
    local translated_filename=filename.2

	for ((i=1; i <= 1000; ++i)); do
		echo "This is the next line of the file: $i" >>rootdir/$filename
	done

	cat mountpoint/$translated_filename >/dev/null &
	cat mountpoint/$translated_filename >/dev/null &
	cat mountpoint/$translated_filename >/dev/null &
	wait

	local execute_number=$(cat "$executed_filename" | wc -l)
    assert [ "$execute_number" -eq 1 ]
}





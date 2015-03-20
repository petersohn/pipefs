#!/bin/bash

cache_limit=0

function test_run()
{
    local filename1=filename1.1
    local filename2=filename2.1
	local translated_filename1=filename1.2
	local translated_filename2=filename2.2

	for ((i=1; i <= 1000; ++i)); do
		echo "$i" >>rootdir/$filename1
	done

	cp rootdir/$filename1 rootdir/$filename2

	cat mountpoint/$translated_filename1 >/dev/null
	cat mountpoint/$translated_filename2 >/dev/null
	cat mountpoint/$translated_filename1 >/dev/null
	cat mountpoint/$translated_filename2 >/dev/null

	local execute_number=$(cat "$executed_filename" | wc -l)
    assert [ "$execute_number" -eq 2 ]
    assert_false lsof rootdir/$filename
}





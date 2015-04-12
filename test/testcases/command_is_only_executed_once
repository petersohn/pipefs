#!/bin/bash

function test_run()
{
    local filename=filename.1
	local translated_filename=filename.2
    local file_contents="This is the first line.
This is the second line.
This is the third line."
    echo "$file_contents" >rootdir/$filename

	cat mountpoint/$translated_filename >/dev/null
	cat mountpoint/$translated_filename >/dev/null
	cat mountpoint/$translated_filename >/dev/null

	local execute_number=$(cat "$executed_filename" | wc -l)
    assert [ "$execute_number" -eq 1 ]
    assert_false lsof rootdir/$filename
}



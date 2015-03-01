#!/bin/bash

function test_run()
{
    local filename=filename.1
	local translated_filename=filename.2
    local file_contents="This is the first line.
This is the second line.
This is the third line."
    echo "$file_contents" >rootdir/$filename

    local expected_file_contents=$(get_expected_file_contents rootdir/$filename)
    local read_file_contents="$(cat mountpoint/$translated_filename)"
    assert [ "$expected_file_contents" == "$read_file_contents" ]
}


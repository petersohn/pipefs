#!/bin/bash

function test_run()
{
	local line1="This is the first line."
	local line2="This is the second line."
    local file_contents="$line1
$line2"
    local expected_file_contents="$line2
$line1"
    local filename=filename.1
	local translated_filename=filename.2

    echo "$file_contents" >rootdir/$filename
    local read_file_contents="$(cat mountpoint/$translated_filename)"
    assert [ "$expected_file_contents" == "$read_file_contents" ]
}


#!/bin/bash

command="sed 's/^/->/'"

function test_run()
{
    local filename=filename.1
	local translated_filename=filename.2

	for ((i=1; i <= 10000; ++i)); do
		echo "This is the next line of the file: $i" >>rootdir/$filename
	done

	local file_contents=$(cat rootdir/$filename)
    local expected_file_contents=$(echo "$file_contents" | eval "$command")

    echo "$file_contents" >rootdir/$filename
    local read_file_contents="$(cat mountpoint/$translated_filename)"
    assert [ "$expected_file_contents" == "$read_file_contents" ]
}



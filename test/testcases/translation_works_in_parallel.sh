#!/bin/bash

function check_file()
{
	local filename="$1"
	local translated_filename="${filename/%.1/.2}"

	local file_contents=$(< "rootdir/$filename")
    local expected_file_contents=$(get_expected_file_contents rootdir/$filename)
    local read_file_contents="$(cat "mountpoint/$translated_filename")"
    assert [ "$expected_file_contents" == "$read_file_contents" ]
    assert_false lsof rootdir/$filename
}

function test_run()
{
    local filename1=filename1.1
    local filename2=filename2.1

	for ((i=1; i <= 10000; ++i)); do
		echo "This is the next line of the first file: $i" >>rootdir/$filename1
		echo "This is the next line of the second file: $i" >>rootdir/$filename2
	done

	check_file $filename1 &
	check_file $filename2 &
	wait
}




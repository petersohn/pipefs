#!/bin/bash

command='bash -c '\''number=$(cat); for ((i=0; i<number; ++i)); do echo content; done'\'

function test_run()
{
	local nowait_filename=nowait.1
	local nowait_translated_filename=nowait.2
	local wait_filename=wait.1
	local wait_translated_filename=wait.2
    echo 1 >rootdir/$nowait_filename
    echo 100000000 >rootdir/$wait_filename

	timeout 0.1 cat mountpoint/$wait_translated_filename >/dev/null
	assert [ $? -ne 0 ]
	read_file_contents=$(cat mountpoint/$nowait_translated_filename)
	assert [ "content" == "$read_file_contents" ]
    assert_false lsof rootdir/$filename
}



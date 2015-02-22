#!/bin/bash

function test_run()
{
    local file_contents="these are the file contents\nsecond line\n"
    local filename=filename.0
	touch rootdir/$filename
    echo -n "$file_contents" >mountpoint/$filename
    local read_file_contents="$(cat rootdir/$filename)"
    assert [ "$file_contents" == "$read_file_contents" ]
}


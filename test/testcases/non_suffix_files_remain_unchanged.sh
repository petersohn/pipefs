#!/bin/bash

function test_run()
{
    local file_contents="these are the file contents
second line"
    local filename=filename.0
    echo "$file_contents" >rootdir/$filename
    local read_file_contents="$(cat mountpoint/$filename)"
    assert [ "$file_contents" == "$read_file_contents" ]
}

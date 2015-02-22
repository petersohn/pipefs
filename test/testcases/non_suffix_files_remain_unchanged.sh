#!/bin/bash

function test_run()
{
    local file_contents="these are the file contents\nsecond line\n"
    local filename=filename.0
    echo -n "$file_contents" >rootdir/$filename
    local read_file_contents="$(cat mountpoint/$filename)"
    assert [ "$file_contents" == "$read_file_contents" ]
}

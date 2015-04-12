#!/bin/bash

function test_run()
{
    local filename=filename.1
    local translated_filename=filename.2

    echo "File contents" >rootdir/$filename
    assert eval "! echo 'This should not be allowed' >mountpoint/$translated_filename"
}


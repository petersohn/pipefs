#!/bin/bash

function test_run()
{
    local filename=filename.1
    echo "File contents" >rootdir/$filename
	assert [ ! -e mountpoint/$filename ]
	assert [ ! -r mountpoint/$filename ]
	assert [ ! -w mountpoint/$filename ]
	assert [ ! -x mountpoint/$filename ]
}





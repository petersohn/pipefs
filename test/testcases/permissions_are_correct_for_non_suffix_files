#!/bin/bash

function test_run()
{
    directory_name=directory
    filename=filename.0

    mkdir rootdir/$directory_name
    echo "File contents" >rootdir/$filename

    assert [ -r mountpoint/$directory_name ]
    assert [ -w mountpoint/$directory_name ]
    assert [ -x mountpoint/$directory_name ]
    assert [ -r mountpoint/$filename ]
    assert [ -w mountpoint/$filename ]
    assert [ ! -x mountpoint/$filename ]

}



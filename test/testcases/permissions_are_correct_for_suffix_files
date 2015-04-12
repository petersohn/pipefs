#!/bin/bash

function test_run()
{
    filename=filename.1
	translated_filename=filename.2

    echo "File contents" >rootdir/$filename

    assert [ -r mountpoint/$translated_filename ]
    assert [ ! -w mountpoint/$translated_filename ]
    assert [ ! -x mountpoint/$translated_filename ]

}




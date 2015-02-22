#!/bin/bash

function test_run()
{
	assert ./pipefs --source-suffix .1 --target-suffix .2 --command cat --root-dir rootdir mountpoint
	sleep 0.1
}


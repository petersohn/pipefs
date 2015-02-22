#!/bin/bash

function test_run()
{
	mkdir rootdir/a
	mkdir rootdir/b
	mkdir rootdir/a/1
	mkdir rootdir/a/2
	touch rootdir/a/file1
	touch rootdir/a/file2
	touch rootdir/a/1/f1
	touch rootdir/a/2/f1

	assert [ -d mountpoint/a ]
	assert [ -d mountpoint/b ]
	assert [ -d mountpoint/a/1 ]
	assert [ -d mountpoint/a/2 ]
	assert [ -f mountpoint/a/file1 ]
	assert [ -f mountpoint/a/file2 ]
	assert [ -f mountpoint/a/1/f1 ]
	assert [ -f mountpoint/a/2/f1 ]
}


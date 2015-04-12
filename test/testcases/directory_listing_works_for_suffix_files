#!/bin/bash

function test_run()
{
	mkdir rootdir/a
	mkdir rootdir/b
	mkdir rootdir/a/1
	mkdir rootdir/a/2
	touch rootdir/a/file1.1
	touch rootdir/a/file2.1
	touch rootdir/a/1/f1.2
	touch rootdir/a/2/f1.2

	directory_contents=$(find mountpoint)
	assert grep -q mountpoint <(echo "$directory_contents")
	assert grep -q mountpoint/a <(echo "$directory_contents")
	assert grep -q mountpoint/b <(echo "$directory_contents")
	assert grep -q mountpoint/a/1 <(echo "$directory_contents")
	assert grep -q mountpoint/a/2 <(echo "$directory_contents")
	assert grep -q mountpoint/a/file1.2 <(echo "$directory_contents")
	assert grep -q mountpoint/a/file2.2 <(echo "$directory_contents")
	assert grep -q mountpoint/a/1/f1.2 <(echo "$directory_contents")
	assert grep -q mountpoint/a/2/f1.2 <(echo "$directory_contents")
	assert [ "$(wc -l < <(echo "$directory_contents"))" == 9 ]
}





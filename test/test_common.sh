#!/bin/bash

function common_init()
{
	mkdir rootdir
	mkdir mountpoint
}

function common_cleanup()
{
	fusermount -u mountpoint
	rm -rf rootdir
	rm -rf mountpoint
}


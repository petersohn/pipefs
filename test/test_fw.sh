#!/bin/bash

function assert()
{
	if ! "$@"; then
		echo "Assertion failed: $@"
		exit 1
	fi
}

function assert_msg()
{
	msg="$1"
	shift
	if ! "$@"; then
		echo "Assertion failed: $msg"
		exit 1
	fi
}


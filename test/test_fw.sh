#!/bin/bash

function assert()
{
	if ! "$@"; then
		echo "Assertion failed: $@"
		exit 1
	fi
}

function assert_false()
{
	if "$@"; then
		echo "Assertion failed: ! $@"
		exit 1
	fi
}


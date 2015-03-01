#!/bin/bash

command=cat

function test_run()
{
    local filename=filename.1
	local translated_filename=filename.2
	string1="some content"
	pos1=0
	len1=${#string1}
	string2="other content"
	pos2=$((pos1+len1))
	len2=${#string2}
	string3="yet another content"
	pos3=$((pos2+len2))
	len3=${#string3}
    local file_contents="${string1}${string2}${string3}"
    echo "$file_contents" >rootdir/$filename

    local expected_file_contents="${string2}${string1}${string3}${string2}"
    local read_file_contents="$("$test_dir/seeker" mountpoint/$translated_filename $pos2 $len2 $pos1 $len1 $pos3 $len3 $pos2 $len2)"
    assert [ "$expected_file_contents" == "$read_file_contents" ]
}



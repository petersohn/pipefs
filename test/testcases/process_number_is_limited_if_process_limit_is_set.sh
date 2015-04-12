#!/bin/bash

executed_filename="$PWD/executed_num"
rm -f "$executed_filename"
command="echo line >>$executed_filename && sleep 0.5 && cat"

custom_options+=(--process-limit=10)

function test_run()
{
    local number_of_processes=20
    local expected_number_of_processes=10
    for ((i=0; i<number_of_processes; ++i)); do
	echo "This is file $i" >rootdir/filename_${i}.1
    done

    for ((i=0; i<number_of_processes; ++i)); do
        cat mountpoint/filename_${i}.2 >/dev/null &
    done
    sleep 0.25
    local process_num=$(cat "$executed_filename" | wc -l)
    assert [ $expected_number_of_processes -eq $process_num ]
    wait
    process_num=$(cat "$executed_filename" | wc -l)
    assert [ $number_of_processes -eq $process_num ]
}





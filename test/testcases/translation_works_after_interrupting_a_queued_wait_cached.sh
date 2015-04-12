#!/bin/bash

executed_filename="$PWD/executed_num"
rm -f "$executed_filename"
command="bash -c '
echo line >>$executed_filename
number=\$(cat)
for ((i=0; i<number; ++i))
	do echo content
done
'"

custom_options+=(--process-limit=10)

function test_run()
{
	local nowait_filename=nowait.1
	local nowait_translated_filename=nowait.2
	local wait_filename=wait.1
	local wait_translated_filename=wait.2
    echo 1 >rootdir/$nowait_filename
    echo 100000000 >rootdir/$wait_filename

	timeout 0.2 cat mountpoint/$wait_translated_filename >/dev/null &
	timeout 0.1 cat mountpoint/$nowait_translated_filename >/dev/null &

	wait
	sleep 0.1
    local executed_count=$(cat "$executed_filename" | wc -l)
    assert [ 2 -eq $executed_count ]

	read_file_contents=$(cat mountpoint/$nowait_translated_filename)
	assert [ "content" == "$read_file_contents" ]
    executed_count=$(cat "$executed_filename" | wc -l)
    assert [ 2 -eq $executed_count ]
}





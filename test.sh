#!/bin/sh

executable=./fwa

file_to_watch=`mktemp` || exit 1
output=`mktemp` || exit 1
./fwa $file_to_watch > $output &
w_pid=$!

expected_output=`mktemp` || exit 1
echo $file_to_watch > $expected_output

echo "some text" > $file_to_watch
sleep 1
kill $w_pid

diff $expected_output $output


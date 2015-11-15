#!/bin/sh

red='\033[0;31m'
green='\033[0;32m'
no_color='\033[0m'

color_print () {
	echo " -> $1$2${no_color}"
}

failed () {
	color_print $red "failed"
	cat $1
	exit 1
}

passed () {
	color_print $green "passed"
}

test_names=`grep ^test_ ./test_definitions.sh | cut -f1 -d " "`
. ./test_definitions.sh
for tst in $test_names; do
	echo -n " -> $tst"
	output_file="$tst.out"
	eval $tst >$output_file 2>&1 || failed $output_file
	rm $output_file
	passed
done


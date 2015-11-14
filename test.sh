#!/bin/sh

output_file="./_test_output"

run_with_parameters_in_the_background () {
	./fwa $* > $output_file &
	return $?
}

run_with_parameters () {
	./fwa $* > $output_file
	return $?
}

test_that_it_prints_out_usage_information_without_files_to_watch () {
	run_with_parameters
	grep -i usage $output_file
	return $?
}

test_that_it_prints_out_a_single_files_name_if_it_is_changed () {
	file_to_watch=`mktemp` || exit 1
	expected_output=`mktemp` || exit 1
	echo $file_to_watch > $expected_output
	run_with_parameters_in_the_background $file_to_watch
	echo "some text" > $file_to_watch
	sleep 1
	diff $expected_output $output_file
	return $?
}


test_that_it_prints_out_usage_information_without_files_to_watch &&
test_that_it_prints_out_a_single_files_name_if_it_is_changed


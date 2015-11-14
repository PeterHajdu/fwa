#!/bin/sh

output_file="./_test_output"

run_with_parameters_in_the_background () {
	echo "./fwa $* > $output_file"
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
	changed_file=`mktemp` || exit 1
	unchanged_file=`mktemp` || exit 1
	run_with_parameters_in_the_background "$unchanged_file $changed_file"
	echo "some text" > $changed_file
	sleep 1
	if grep $unchanged_file $output_file; then
		return 1
	fi

	if grep $changed_file $output_file; then
		return 0
	fi

	return 1
}


test_that_it_prints_out_usage_information_without_files_to_watch &&
test_that_it_prints_out_a_single_files_name_if_it_is_changed


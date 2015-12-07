#!/bin/sh

output_file="./_test_output"

run_with_parameters_in_the_background () {
	echo "./fwa $* > $output_file"
	./fwa $* > $output_file &
	sleep 1
	return 0
}

kill_background_fwa () {
	pkill fwa
}

run_with_parameters () {
	./fwa $* > $output_file
	return $?
}

test_that_it_stops_if_its_unable_to_open_a_file () {
	existing_file=`mktemp` || exit 1
	run_with_parameters "$existing_file definitely_not_existing_file"
	return 0
}

test_that_it_prints_out_usage_information_without_files_to_watch () {
	run_with_parameters
	grep -i usage $output_file
	return $?
}

test_that_it_prints_out_usage_information_if_the_dash_h_option_is_passed_in () {
	run_with_parameters "-h"
	grep -i usage $output_file
	return $?
}

test_that_it_prints_out_usage_information_if_the_help_option_is_passed_in () {
	run_with_parameters "--help"
	grep -i usage $output_file
	return $?
}

test_that_it_prints_out_a_single_files_name_if_it_is_changed () {
	changed_file=`mktemp` || exit 1
	unchanged_file=`mktemp` || exit 1
	run_with_parameters_in_the_background "$unchanged_file $changed_file"
	echo "some test" > $changed_file
	sleep 1
	kill_background_fwa
	if grep $unchanged_file $output_file; then
		return 1
	fi

	if grep $changed_file $output_file; then
		return 0
	fi

	return 1
}

test_that_it_prints_out_each_changed_files_name () {
	file_1=`mktemp` || exit 1
	file_2=`mktemp` || exit 1
	run_with_parameters_in_the_background "$file_1 $file_2"
	echo "some test" > $file_1
	echo "some test" > $file_2
	sleep 1
	kill_background_fwa
	if grep $file_1 $output_file && grep $file_2 $output_file; then
		return 0
	fi

	return 1
}

test_that_it_prints_out_fast_file_changes_only_once () {
	file_1=`mktemp` || exit 1
	run_with_parameters_in_the_background "$file_1"
	echo "some test" > $file_1
	echo "some test" > $file_1
	sleep 1
	kill_background_fwa
	number_of_changes=`grep $file_1 $output_file | wc -l`
	if [ $number_of_changes -lt 2 ]; then
		return 0
	fi

	return 1
}

test_that_it_continues_to_watch_files_after_deletion_and_recreation () {
	file=`mktemp` || exit 1
	run_with_parameters_in_the_background "$file"
	rm -f $file
	touch $file
	sleep 1
	touch $file
	sleep 1
	kill_background_fwa
	number_of_changes=`grep $file $output_file | wc -l`
	if [ $number_of_changes -lt 2 ]; then
		return 1
	fi

	return 0
}

test_that_it_continues_to_watch_files_after_deletion () {
	file_1=`mktemp` || exit 1
	file_2=`mktemp` || exit 1
	run_with_parameters_in_the_background "$file_1 $file_2"
	rm -f $file_1
	sleep 2
	touch $file_2
	sleep 1
	kill_background_fwa
	number_of_changes=`grep $file_2 $output_file | wc -l`
	if [ $number_of_changes -lt 1 ]; then
		return 1
	fi

	return 0
}


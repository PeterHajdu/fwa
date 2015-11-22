/*
 * Copyright (c) 2015, Peter Ferenc Hajdu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

void usage();
int create_queue();
size_t parse_options(int, char*[]);
struct kevent* allocate_event_memory(size_t);
size_t set_up_events_to_watch(struct kevent *, size_t, char*[]);
void set_output_buffer();
void handle_events(int, struct kevent*, size_t);

int main(int argc, char **argv) {
	const size_t number_of_files = parse_options(argc, argv);
	const int queue = create_queue();
	struct kevent *events_to_monitor = allocate_event_memory(
			number_of_files);
	const size_t number_of_events = set_up_events_to_watch(
			events_to_monitor,
			number_of_files,
			argv);
	set_output_buffer();
	handle_events(queue, events_to_monitor, number_of_events);
	return 0;
}

void usage() {
	printf(
		"usage: fwa [options] <list of files to watch>\n"
		"\noptions:\n"
		"\t-h --help\tPrint out this message.\n");
	exit(1);
}

int create_queue() {
	const int queue = kqueue();
	if (queue < 0)
		err(1, "Unable to create kernel queue." );
	return queue;
}

size_t parse_options(int argc, char* argv[]) {
	size_t number_of_files = argc - 1;
	if (pledge("stdio rpath", NULL) == -1)
		err(6, "pledge");
	if (argc < 2)
		usage();
	if (0 == strncmp(argv[1], "-h", 2) || 0 == strncmp(argv[1], "--help", 6))
		usage();
	if (number_of_files < 1)
		usage();
	return number_of_files;
}

struct kevent* allocate_event_memory(size_t number_of_files) {
	struct kevent * const ptr = reallocarray(
			NULL,
			number_of_files,
			sizeof(struct kevent));
	if (ptr == NULL)
		err(2, "Unable to allocate event memory.");
	return ptr;
}

struct event_descriptor {
	const char* filename;
	int numbers_triggered;
};

struct event_descriptor* create_event_descriptor(const char* filename) {
	struct event_descriptor* descriptor = malloc(sizeof(struct event_descriptor));
	if (descriptor == NULL)
		err(4, "Unable to allocate memory for descriptor.");
	descriptor->filename=filename;
	descriptor->numbers_triggered=0;
	return descriptor;
}

void mark_event(struct event_descriptor* descriptor) {
	descriptor->numbers_triggered++;
}

size_t set_up_events_to_watch(struct kevent *events, size_t number_of_files, char* argv[]) {
	const unsigned int vnode_events =
		NOTE_DELETE |
		NOTE_WRITE |
		NOTE_EXTEND |
		NOTE_ATTRIB |
		NOTE_LINK |
		NOTE_RENAME |
		NOTE_REVOKE;
	int i = 0;
	int event_slot = 0;
	for(; i < number_of_files; i++) {
		char* const filename = argv[i+1];
		const int event_fd = open(filename, O_RDONLY);
		if (event_fd<0)
			err(5, "Unable to open file: %s", filename);
		EV_SET(
			&events[event_slot],
			event_fd,
			EVFILT_VNODE,
			EV_ADD | EV_CLEAR,
			vnode_events,
			0,
			create_event_descriptor(filename));
		event_slot++;
	}

	if (pledge("stdio", NULL) == -1)
		err(7, "pledge");

	return event_slot;
}

void set_output_buffer() {
  static char line_buffer[512];
  setvbuf(stdout, line_buffer, _IOLBF, sizeof(line_buffer));
}

void report_and_cleanup_events(struct kevent* monitored_events, size_t number_of_events) {
	size_t i = 0;
	for(; i < number_of_events; i++)
	{
		struct event_descriptor* descriptor =
			monitored_events[i].udata;
		if (descriptor->numbers_triggered == 0)
			continue;
		printf("%s\n", descriptor->filename);
		descriptor->numbers_triggered = 0;
	}
}

void handle_events(int queue, struct kevent* events_to_monitor, size_t number_of_events) {
	struct kevent event_data[1];
	struct timespec timeout;
	timeout.tv_sec=0;
	timeout.tv_nsec=500000000;
	while (1) {
		int event_count = kevent(
				queue,
				events_to_monitor,
				number_of_events,
				event_data,
				1,
				&timeout);
		if(event_count < 0)
			err(3, "Error occured while waiting for events.");
		if(event_count > 0) {
			mark_event(event_data[0].udata);
			continue;
		}

		report_and_cleanup_events(events_to_monitor, number_of_events);
	}
}


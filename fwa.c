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
#include <time.h>

void usage(void);
int create_queue(void);
size_t parse_options(int, char*[]);
struct kevent* allocate_event_memory(size_t);
size_t set_up_events_to_watch(int, struct kevent *, size_t, char*[]);
void set_output_buffer(void);
void handle_events(int, struct kevent*, size_t);

int main(int argc, char **argv) {
	int queue = create_queue();
	size_t number_of_files = 0;
	struct kevent *events_to_monitor = NULL;

	if (pledge("stdio rpath", NULL) == -1)
		err(6, "pledge");

	set_output_buffer();
	number_of_files = parse_options(argc, argv);
	events_to_monitor = allocate_event_memory(number_of_files);
	number_of_files = set_up_events_to_watch(queue, events_to_monitor, number_of_files, argv);

	handle_events(queue, events_to_monitor, number_of_files);
	return 0;
}

void usage(void) {
	printf(
		"usage: fwa [options] <list of files to watch>\n"
		"\noptions:\n"
		"\t-h --help\tPrint out this message.\n");
	exit(1);
}

size_t parse_options(int argc, char* argv[]) {
	size_t number_of_files = argc - 1;
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
	unsigned int flags;
};

struct event_descriptor* create_event_descriptor(const char* filename) {
	struct event_descriptor* descriptor = malloc(sizeof(struct event_descriptor));
	if (descriptor == NULL)
		err(4, "Unable to allocate memory for descriptor.");
	descriptor->filename=filename;
	descriptor->flags=0;
	return descriptor;
}

void mark_event(struct kevent* event) {
	struct event_descriptor* descriptor = event->udata;
	descriptor->flags|=event->fflags;
}

static const unsigned int monitored_events_mask =
	NOTE_DELETE |
	NOTE_WRITE |
	NOTE_EXTEND |
	NOTE_ATTRIB |
	NOTE_LINK |
	NOTE_RENAME |
	NOTE_REVOKE;

static const unsigned short clear_and_add = EV_ADD | EV_CLEAR;

int try_to_open_file(const char* filename){
	int i;
	struct timespec delay;
	delay.tv_sec=0;
	delay.tv_nsec=100000000;
	for (i=0; i<10; ++i) {
		const int fd = open(filename, O_RDONLY);
		if (fd!=-1)
			return fd;
		nanosleep(&delay, NULL);
	}

	return -1;
}

size_t set_up_events_to_watch(
		int queue,
		struct kevent *events,
		size_t number_of_files,
		char* argv[]) {
	int i = 0;
	int event_slot = 0;
	for(; i < number_of_files; i++) {
		char* const filename = argv[i+1];
		const int event_fd = try_to_open_file(filename);
		if (event_fd<0)
			err(5, "Unable to open file: %s", filename);
		EV_SET(
			&events[event_slot],
			event_fd,
			EVFILT_VNODE,
			clear_and_add,
			monitored_events_mask,
			0,
			create_event_descriptor(filename));
		if (-1 == kevent( queue, &events[event_slot], 1, NULL, 0, NULL))
			err(7, "Unable to register event filter for file: %s", filename);
		event_slot++;
	}
	return event_slot;
}

void set_output_buffer(void) {
  static char line_buffer[512];
  setvbuf(stdout, line_buffer, _IOLBF, sizeof(line_buffer));
}

int is_delete_event(struct event_descriptor* descriptor) {
	const unsigned int delete_events =
		NOTE_DELETE |
		NOTE_RENAME |
		NOTE_REVOKE;
	return descriptor->flags & delete_events;
}


void fix_descriptor_if_deleted(int queue, struct kevent* event){
	int fd = event->ident;
	struct event_descriptor* descriptor = event->udata;

	if (!is_delete_event(descriptor))
		return;
	if (-1 == close(fd))
		warn("Unable to close file.");
	fd = try_to_open_file(descriptor->filename);
	if (-1 == fd)
		return;

	EV_SET(
		event,
		fd,
		EVFILT_VNODE,
		clear_and_add,
		monitored_events_mask,
		0,
		event->udata);
	if (-1 == kevent( queue, event, 1, NULL, 0, NULL))
		warn("Unable to reenable event on file.");
}

void report_and_cleanup_events(
		int queue,
		struct kevent* monitored_events,
		size_t number_of_events) {
	size_t i = 0;
	for(; i < number_of_events; i++)
	{
		struct event_descriptor* descriptor = monitored_events[i].udata;
		if (descriptor->flags == 0)
			continue;
		printf("%s\n", descriptor->filename);
		fix_descriptor_if_deleted(queue, &monitored_events[i]);
		descriptor->flags = 0;
	}
}

int create_queue(void) {
	const int queue = kqueue();
	if (queue < 0)
		err(1, "Unable to create kernel queue." );
	return queue;
}

void handle_events(int queue, struct kevent* events_to_monitor, size_t number_of_events) {
	struct kevent event_data;
	struct timespec timeout;
	timeout.tv_sec=0;
	timeout.tv_nsec=500000000;
	while (1) {
		int event_count = kevent(
				queue,
				NULL,
				0,
				&event_data,
				1,
				&timeout);
		if(event_count < 0)
			err(3, "Error occured while waiting for events.");
		if(event_count > 0) {
			mark_event(&event_data);
			continue;
		}

		report_and_cleanup_events(queue, events_to_monitor, number_of_events);
	}
}


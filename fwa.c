#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>

void usage();
int create_queue();
size_t get_number_of_files();
struct kevent* allocate_event_memory(size_t);
size_t set_up_events_to_watch( struct kevent *, size_t, char*[] );
void set_output_buffer();
void handle_events( int, struct kevent*, size_t );

int main(int argc, char *argv[]) {
	const size_t number_of_files = get_number_of_files(argc);
	const int queue = create_queue();
	struct kevent *events_to_monitor = allocate_event_memory(
			number_of_files );
	const size_t number_of_events = set_up_events_to_watch(
			events_to_monitor,
			number_of_files,
			argv );
	set_output_buffer();
	handle_events( queue, events_to_monitor, number_of_events );
	return 0;
}

void usage() {
	printf("usage: fwa <list of files to watch>\n");
	exit(1);
}

int create_queue() {
	const int queue = kqueue();
	if ( queue < 0 )
		err( 1, "Unable to create kernel queue." );
	return queue;
}

size_t get_number_of_files(int argc) {
	const size_t number_of_files = argc - 1;
	if (number_of_files < 1)
		usage();
	return number_of_files;
}

struct kevent* allocate_event_memory(size_t number_of_files) {
	struct kevent * const ptr = reallocarray(
			NULL,
			number_of_files,
			sizeof( struct kevent ));
	if ( ptr == NULL )
		err(2, "Unable to allocate event memory.");
	return ptr;
}

struct event_descriptor {
	const char* filename;
	int numbers_triggered;
};

struct event_descriptor* create_event_descriptor( const char* filename ) {
	struct event_descriptor* descriptor =
		malloc( sizeof( struct event_descriptor ) );
	if ( descriptor == NULL )
		err(4, "Unable to allocate memory for descriptor.");
	descriptor->filename=filename;
	descriptor->numbers_triggered=0;
	return descriptor;
}

void mark_event( struct event_descriptor* descriptor ) {
	descriptor->numbers_triggered++;
}

size_t set_up_events_to_watch( struct kevent *events, size_t number_of_files, char* argv[] ) {
	static const unsigned int vnode_events =
		NOTE_DELETE |
		NOTE_WRITE |
		NOTE_EXTEND |
		NOTE_ATTRIB |
		NOTE_LINK |
		NOTE_RENAME |
		NOTE_REVOKE;
	int i = 0;
	int event_slot = 0;
	for( ; i < number_of_files; i++ ) {
		char* const filename = argv[i+1];
		const int event_fd = open( filename, O_RDONLY );
		if (event_fd<0) {
			fprintf( stderr, "Unable to open file: %s\n", filename );
			continue;
		}
		EV_SET(
			&events[event_slot],
			event_fd,
			EVFILT_VNODE,
			EV_ADD | EV_CLEAR,
			vnode_events,
			0,
			create_event_descriptor( filename ) );
		event_slot++;
	}
	return event_slot;
}

void set_output_buffer() {
  static char line_buffer[ 512 ];
  setvbuf(stdout, line_buffer, _IOLBF, sizeof(line_buffer));
}

void report_and_cleanup_events( struct kevent* monitored_events, size_t number_of_events ) {
	size_t i = 0;
	for( ; i < number_of_events; i++ )
	{
		struct event_descriptor* descriptor =
			monitored_events[i].udata;
		if ( descriptor->numbers_triggered == 0 )
			continue;
		printf( "%s\n", descriptor->filename );
		descriptor->numbers_triggered = 0;
	}
}

void handle_events( int queue, struct kevent* events_to_monitor, size_t number_of_events ) {
	struct kevent event_data[1];
	struct timespec timeout;
	timeout.tv_sec=0;
	timeout.tv_nsec=500000000;
	while ( 1 ) {
		int event_count = kevent(
				queue,
				events_to_monitor,
				number_of_events,
				event_data,
				1,
				&timeout );
		if( event_count < 0 )
			err( 3, "Error occured while waiting for events." );
		if( event_count > 0 ) {
			mark_event( event_data[0].udata );
			continue;
		}

		report_and_cleanup_events( events_to_monitor, number_of_events );
	}
}


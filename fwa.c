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

int main(int argc, char *argv[]) {
	const size_t number_of_files = get_number_of_files(argc);
	const int queue = create_queue();
	struct kevent *events_to_monitor = allocate_event_memory(number_of_files);
	const size_t number_of_events = set_up_events_to_watch( events_to_monitor, number_of_files, argv );
	(void)queue;
	(void)number_of_files;
	(void)number_of_events;

	printf("%s\n%s\n", argv[1], argv[2]);
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

struct kevent* allocate_event_memory(size_t number_of_files)
{
	struct kevent * const ptr = reallocarray(NULL, number_of_files, sizeof(struct kevent));
	if ( kevent == NULL )
		err(2, "Unable to allocate event memory.");
	return ptr;
}

size_t set_up_events_to_watch( struct kevent *events, size_t number_of_files, char* argv[] )
{
	static const unsigned int vnode_events = NOTE_DELETE |  NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;
	int i = 0;
	int event_slot = 0;
	for( ; i < number_of_files; i++ )
	{
		char* const filename = argv[i+1];
		int event_fd = open( filename, O_RDONLY );
		if (event_fd<0) {
			fprintf( stderr, "Unable to open file: %s\n", filename );
			continue;
		}
		EV_SET( &events[event_slot], event_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, vnode_events, 0, filename );
		event_slot++;
	}
	return event_slot;
}

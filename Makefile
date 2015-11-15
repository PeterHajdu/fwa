SOURCES=fwa.c
EXECUTABLE=fwa
TEST=test.sh
CFLAGS=-Wall -Werror -pedantic -g
DESTDIR?=/usr/local/bin

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

test: $(EXECUTABLE) $(TEST)
	$(TEST)

install: $(EXECUTABLE)
	install $(EXECUTABLE) $(DESTDIR)

auto_test:
	fwa *.c Makefile *.sh | while read; do make test; done

clean:
	rm -f $(EXECUTABLE)

.PHONY: clean test install auto_test


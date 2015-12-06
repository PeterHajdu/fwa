PREFIX ?= /usr/local
SOURCES=fwa.c
EXECUTABLE=fwa
TEST=test.sh
CFLAGS=-Wall -Werror -pedantic -g

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

test: $(EXECUTABLE) $(TEST)
	$(TEST)

install: $(EXECUTABLE)
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	install $(EXECUTABLE) ${DESTDIR}${PREFIX}/bin

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/$(EXECUTABLE)

auto_test:
	fwa *.c Makefile *.sh | while read; do make test; done

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: clean test install auto_test all


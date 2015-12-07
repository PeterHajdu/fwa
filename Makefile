PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/man
SOURCES=fwa.c
EXECUTABLE=fwa
TEST=test.sh
CFLAGS=-Wall -Werror -pedantic -g

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

test: $(EXECUTABLE) $(TEST)
	sh $(TEST)

install: $(EXECUTABLE)
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	install $(EXECUTABLE) ${DESTDIR}${PREFIX}/bin
	install -m 644 fwa.1 ${DESTDIR}${MANPREFIX}/man1

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/$(EXECUTABLE)
	rm ${DESTDIR}${MANPREFIX}/man1/fwa.1

auto_test:
	fwa *.c Makefile *.sh | while read; do make test; done

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: clean test install auto_test all


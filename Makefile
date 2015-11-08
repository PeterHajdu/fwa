SOURCES=fwa.c
EXECUTABLE=fwa
TEST=test.sh
CFLAGS=-Wall -Werror -pedantic

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

test: $(EXECUTABLE) $(TEST)
	$(TEST)

clean:
	rm -f $(EXECUTABLE)

.PHONY: clean test


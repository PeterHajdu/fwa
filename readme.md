# Introduction

fwa is a simple file change notifier keeping unix philosophy in mind.
It is intended to be a clean and simple application to watch file changes on
systems using kqueue.
It does not execute any commands, just prints out the changed file's name.
To handle the events, read fwa's output and act as you wish.

# Installation

```sh
git clone http://github.com/peterhajdu/fwa
cd fwa
make install
```

# Examples

## Simply execute one command

```sh
fwa * | while read; do make test; done
```

## Execute commands specific for a file type

```sh
fwa * | while true; do handle; done
```

Where handle might look like this.

```bash
#!/bin/sh
read line

if echo $line | grep .c; then
	echo Handling c file.
	make test
fi

if echo $line | grep .md; then
	echo Handling markdown file.
	cat $line
fi
```


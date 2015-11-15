#Introduction

fwa is a simple file change notifier keeping unix philosophy in mind.  It is intended to be a clean simple application for OpenBSD.

#How to start tests automatically with the aid of fwa

```
fwa * | while read; do make test; done
```


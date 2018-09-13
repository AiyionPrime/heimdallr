PREFIX = /usr

CC = gcc
CFLAGS = -Wall -Werror -DVERSION=\"$(GIT_VERSION)\"

LDLIBS = -lcurl -ljson-c -lcrypto -lssh
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include
TESTLIBS = -lcheck

MAN = heimdallr.1
SOURCES = heimdallr.c config.c sshserver.c github.c
OUT = heimdallr
OBJ = $(src:.c=.o)

GIT_VERSION := $(shell git describe --dirty --always --tags)

all: build

build: $(SOURCES) compiler_flags
	$(CC) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OUT) $(MAN).gz
	rm -f test/*.c
	rm -f test/check
	rm -f all_tests.c

.PHONY: install-bin
install-bin: build
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(OUT) $(DESTDIR)$(PREFIX)/bin/$(OUT)

.PHONY: doc
doc:
	gzip -c man/$(MAN) > heimdallr.1.gz

.PHONY: install-doc
install-doc: doc
	cp $(MAN).gz $(DESTDIR)$(PREFIX)/share/man/man1/

.PHONY: install
install: install-bin install-doc

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(OUT)
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/$(MAN).gz

.PHONY: force
compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

prepare-check:
	checkmk test/config-test.check > all_tests.c

compile-check: prepare-check
	$(CC) -o test/check $(INCLUDE) $(CFLAGS) $(LDFLAGS) config.c all_tests.c $(LDLIBS) $(TESTLIBS)

.PHONY: check
check: compile-check
	./test/check

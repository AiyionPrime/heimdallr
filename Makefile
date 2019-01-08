PREFIX = /usr

CC = gcc
CFLAGS = -Wall -Werror -DGIT_VERSION=\"$(GIT_VERSION)\" -DVERSION=\"$(VERSION)\"

MOCKS_SSHSERVER = fopen ssh_channel_read ssh_pki_import_pubkey_file ssh_print_hash
MOCKS_CONFIG = mkdir
MOCKS_GITHUB = getline printf

LDLIBS = -lcurl -ljson-c -lcrypto -lssh
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include
TESTLIBS = -lcmocka
TESTFLAGS =

TESTFLAGS_CONFIG += $(foreach MOCK,$(MOCKS_CONFIG),-Wl,--wrap=$(MOCK))
TESTFLAGS_GITHUB += $(foreach MOCK,$(MOCKS_GITHUB),-Wl,--wrap=$(MOCK))
TESTFLAGS_SSHSERVER += $(foreach MOCK,$(MOCKS_SSHSERVER),-Wl,--wrap=$(MOCK))

MAN = heimdallr.1
SOURCES = heimdallr.c config.c sshserver.c github.c
OUT = heimdallr
OBJ = $(src:.c=.o)

VERSION := "v1.0.3"
GIT_VERSION := $(shell git describe --dirty --always --tags)

all: build

build: $(SOURCES) compiler_flags
	$(CC) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OUT) $(MAN).gz
	rm -f test/test_config
	rm -f test/test_github
	rm -f test/test_sshserver

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

compile-check-config:
	$(CC) -o test/test_config $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_config.c config.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_CONFIG) $(TESTFLAGS)

compile-check-github:
	$(CC) -o test/test_github $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_github.c github.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_GITHUB) $(TESTFLAGS)

compile-check-sshserver:
	$(CC) -o test/test_sshserver $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_sshserver.c sshserver.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_SSHSERVER) $(TESTFLAGS)


.PHONY: check
check: clean compile-check-config compile-check-github compile-check-sshserver
	./test/test_config
	./test/test_github
	./test/test_sshserver

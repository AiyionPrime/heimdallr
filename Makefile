PREFIX = /usr

CC = gcc
CFLAGS = -Wall -Werror -DGIT_VERSION=\"$(GIT_VERSION)\" -DVERSION=\"$(VERSION)\"

MOCKS_SSHSERVER = fopen ssh_channel_read ssh_pki_import_pubkey_file ssh_print_hash
MOCKS_CONFIG = mkdir
MOCKS_GITHUB = getline get_githubuser_dir import_pubkey_comment _test_malloc opendir readdir ssh_pki_import_pubkey_file closedir printf
MOCKS_KEYS = printf _test_malloc
MOCKS_KEYS_RCO = fileno fopen fread fstat _test_malloc read_comment_oneline

LDLIBS = -lcurl -ljson-c -lcrypto -lssh
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include
TESTLIBS = -lcmocka
TESTFLAGS =

TESTFLAGS_CONFIG += $(foreach MOCK,$(MOCKS_CONFIG),-Wl,--wrap=$(MOCK))
TESTFLAGS_KEYS += -DUNIT_TESTING $(foreach MOCK,$(MOCKS_KEYS),-Wl,--wrap=$(MOCK))
TESTFLAGS_KEYS_RCO += -DUNIT_TESTING $(foreach MOCK,$(MOCKS_KEYS_RCO),-Wl,--wrap=$(MOCK))
TESTFLAGS_GITHUB += $(foreach MOCK,$(MOCKS_GITHUB),-Wl,--wrap=$(MOCK))
TESTFLAGS_SSHSERVER += $(foreach MOCK,$(MOCKS_SSHSERVER),-Wl,--wrap=$(MOCK))

MAN = heimdallr.1
SOURCES = heimdallr.c config.c keys.c keys_fileops.c sshserver.c github.c
OUT = heimdallr
OBJ = $(src:.c=.o)

VERSION := "v1.1"
GIT_VERSION := $(shell git describe --dirty --always --tags)

all: build

build: $(SOURCES) compiler_flags
	$(CC) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OUT) $(MAN).gz
	rm -f test/test_config
	rm -f test/test_keys
	rm -f test/test_keys_m_rco
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

compile-check-keys:
	$(CC) -o test/test_keys $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_keys.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_KEYS) $(TESTFLAGS)

compile-check-keys-rco:
	$(CC) -o test/test_keys_m_rco $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_keys_m_rco.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_KEYS_RCO) $(TESTFLAGS)

compile-check-github:
	$(CC) -o test/test_github $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_github.c keys.c keys_fileops.c github.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_GITHUB) $(TESTFLAGS)

compile-check-sshserver:
	$(CC) -o test/test_sshserver $(INCLUDE) $(CFLAGS) $(LDFLAGS) test/test_sshserver.c sshserver.c $(LDLIBS) $(TESTLIBS) $(TESTFLAGS_SSHSERVER) $(TESTFLAGS)

.PHONY: compile-check
compile-check: clean compile-check-config compile-check-github compile-check-keys compile-check-keys-rco compile-check-sshserver

.PHONY: check
check: compile-check
	./test/test_config
	./test/test_github
	./test/test_keys
	./test/test_keys_m_rco
	./test/test_sshserver

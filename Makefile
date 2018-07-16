PREFIX = /usr

CXX = gcc
CFLAGS = -Wall -Werror -DVERSION=\"$(GIT_VERSION)\"

LDLIBS = -lcurl -ljson-c -lcrypto -lssh
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include

SOURCES = heimdallr.c config.c sshserver.c github.c
OUT = heimdallr
OBJ = $(src:.c=.o)

GIT_VERSION := $(shell git describe --dirty --always --tags)

all: build

build: $(SOURCES) compiler_flags
	$(CXX) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OUT)

.PHONY: install
install: heimdallr
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/$(OUT)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(OUT)

.PHONY: force
compiler_flags: force
	echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

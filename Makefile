PREFIX = /usr

CXX = gcc
CFLAGS = -Wall -Werror

LDLIBS = -lcurl -ljson-c
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include

SOURCES = heimdallr.c
OUT = heimdallr
OBJ = $(src:.c=.o)

all: build

build: $(SOURCES)
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

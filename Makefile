CXX = gcc
CFLAGS = -Wall -Werror

LDLIBS = -lcurl -ljson-c
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include

SOURCES = kraken.c
OUT = kraken

all: build

build: $(SOURCES)
	$(CXX) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

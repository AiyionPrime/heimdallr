CXX = gcc
CFLAGS = -Wall -Werror

LDLIBS = -lcurl -ljson-c
#LDFLAGS = -Lusr/local/lib 
#INCLUDE = -Iusr/local/include

SOURCES = kraken.c
OUT = kraken
OBJ = $(src:.c=.o)

all: build

build: $(SOURCES)
	$(CXX) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OUT)

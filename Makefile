CC = clang

CFLAGS += -O2 -std=c11 -ggdb -fstack-protector-all -Wl,-z,relro -Wl,-z,now -fPIE -pie
CFLAGS += -Werror -Weverything -Wno-unused-macros -Wno-padded
CFLAGS += -D_DEFAULT_SOURCE

all: msntpc

msntpc: src/msntpc.c src/msntpc.h
	$(CC) $(CFLAGS) -o $@ src/msntpc.c

clean:
	-rm -f msntpc

.PHONY: all clean

OBJS=obj/dev.o obj/merrno.o obj/print.o obj/service.o
CFLAGS=-Iinclude -lpthread

all: bin/server

bin/server: src/server.c $(OBJS)
	cc -o $@ $^ $(CFLAGS)

obj/service.o: src/service.c include/merrno.h
	cc -o $@ -c $< $(CFLAGS)

obj/dev.o: src/dev.c include/dev.h include/merrno.h
	cc -o $@ -c $< $(CFLAGS)

obj/merrno.o: src/merrno.c include/merrno.h
	cc -o $@ -c $< $(CFLAGS)

obj/print.o: src/print.c include/print.h include/huffman.h
	cc -o $@ -c $< $(CFLAGS)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) bin/server

.PHONY: clean distclean all
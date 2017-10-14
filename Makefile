OBJS=obj/main.o obj/dev.o obj/merrno.o obj/print.o obj/server.o

bin/main: $(OBJS)
	cc -o $@ $^

obj/main.o: src/main.c include/merrno.h
	cc -o $@ -c $< -Iinclude

obj/dev.o: src/dev.c include/dev.h include/merrno.h
	cc -o $@ -c $< -Iinclude

obj/merrno.o: src/merrno.c include/merrno.h
	cc -o $@ -c $< -Iinclude

obj/print.o: src/print.c include/print.h include/huffman.h
	cc -o $@ -c $< -Iinclude

obj/server.o: src/server.c include/server.h
	cc -o $@ -c $< -Iinclude

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) bin/main

.PHONY: clean distclean
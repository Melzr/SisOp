CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wmissing-prototypes -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := ps timeout ls

all: $(PROGS)

ps: ps.o
ls: ls.c
timeout: timeout.o
	cc timeout.o -o timeout -lrt
	
timeout.o: timeout.c
	cc -ggdb3 -O2 -Wall -Wextra -std=c11 -Wmissing-prototypes -Wvla -D_DEFAULT_SOURCE -c -o timeout.o timeout.c -lrt	

format: .clang-files ../.clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean format

#valgrind --track-fds=yes --leak-check=full --show-leak-kinds=all --trace-children=yes
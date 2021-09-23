CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wmissing-prototypes -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := ps

all: $(PROGS)

ps: ps.o

format: .clang-files ../.clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean format

# valgrind --track-fds=yes --leak-check=full --show-leak-kinds=all --trace-children=yes ./
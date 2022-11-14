CFLAGS ?=
CFLAGS += `pkg-config --static --cflags sqlite3`
CFLAGS += -g -DDEBUG
CFLAGS += -Iinclude

LDFLAGS ?=
LDFLAGS += `pkg-config --static --libs sqlite3`
all: test_shell

SOURCES = src/test_shell.c
SOURCES += src/spect.c
SOURCES += src/spect_config.c
SOURCES += src/spect_network_node_list.c

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

.PHONY=all

all: database.db test_shell

test_shell: $(OBJS)
	$(CC) -o test_shell $^ $(LDFLAGS)

%.o:src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

database.db: deps
	mix ecto.migrate

deps:
	mix deps.get

run: test_shell
	./test_shell

valgrind: test_shell
	valgrind --leak-check=full --show-leak-kinds=all ./test_shell

clean:
	$(RM) test_shell $(OBJS)

dirclean: clean
	$(RM) database.db

PROJECT = nanocad
CC = clang
RM = rm -f
GDB = gdb
CFLAGS = -Wall -std=gnu99 $(shell sdl2-config --cflags)
LDFLAGS = -lm -lreadline $(shell sdl2-config --libs)
OBJECTS = src/app.o src/nanocad.o src/graphics.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g3 -DDEBUG
debug: $(PROJECT)
	$(GDB)

memcheck: CFLAGS += -g3 -DDEBUG -DMEMCHECK
memcheck: $(PROJECT)
	valgrind --tool=memcheck --leak-check=yes --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log ./$(PROJECT) test.ncad
	cat valgrind.log

clean:
	$(RM) -r src/*.o
	$(RM) $(PROJECT)
	$(RM) valgrind.log


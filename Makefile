PROJECT = nanocad
CC = gcc
RM = rm -f
CFLAGS = -Wall -std=gnu99 $(shell sdl2-config --cflags)
LDFLAGS = -lreadline $(shell sdl2-config --libs)
OBJECTS = src/app.o src/nanocad.o src/graphics.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

debug: CFLAGS += -g3 -DDEBUG
debug: $(PROJECT)
	./$(PROJECT) test.ncad

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r src/*.o
	$(RM) $(PROJECT)


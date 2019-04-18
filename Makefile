PROJECT = nanocad
CC = gcc
RM = rm -f
CFLAGS = -Wall -std=gnu99
LIBS = -lreadline
#HEADERS = gnuplot_i/gnuplot_i.h
OBJECTS = src/app.o src/nanocad.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

debug: CFLAGS += -g3 -DDEBUG
debug: $(PROJECT)
	./$(PROJECT) test.ncad

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r src/*.o
	$(RM) $(PROJECT)


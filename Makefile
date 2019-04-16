PROJECT = nanocad
CC = gcc
RM = rm -f
CFLAGS = -Wall -std=gnu99
LIBS = -lreadline
#HEADERS = gnuplot_i/gnuplot_i.h
OBJECTS = src/app.o src/nanocad.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CXXFLAGS) $(OBJECTS) -o $@ $(LIBS)

debug: CXXFLAGS += -g3 -DDEBUG
debug: $(PROJECT)
	./$(PROJECT)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r src/*.o
	$(RM) $(PROJECT)


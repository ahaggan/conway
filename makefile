CFLAGS =  -O4 -Wall -pedantic -std=c99 -lm
INCS = neillsdl2.h 
TARGET = working_hash
SOURCES =  neillsdl2.c $(TARGET).c
LIBS = -lSDL2
CC = gcc


all: $(TARGET)

$(TARGET): $(SOURCES) $(INCS)
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

run: all
	$(TARGET) 

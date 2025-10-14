# Makefile for the "FRAKEN" TTSM system monitor program on FreeBSD

# Variables
# When we're done, we should prune this to make it look like I actually
# 	know what it is that I'm doing.  -Fraken.
CC = cc
#add -Ofast to the CFLAGS list or one of the other -blah to improve speed.  -Fraken.
# You can remove ALL -O flags if you want though.  No worries. :)
# Let's also try -march=native, -ffast-math, -finline-functions
CFLAGS = -Wall -Wextra -I/usr/local/include
LDFLAGS = -lpthread -ldevstat -I/usr/local/include -L/usr/local/lib -lX11

SOURCES = main.c cpu.c network.c disk.c swap.c globals.c graphics.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ttsm

# Default target: builds the executable
all: $(EXECUTABLE)

# Rule to link the object files into the final executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

# Rule to compile each .c file into a .o object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: removes the compiled files
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

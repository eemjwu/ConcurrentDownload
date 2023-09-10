# Compiler to use
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -std=c++11

# Libraries to link against
LIBS = -lcurl -lpthread

# Name of the executable
TARGET = downloader

# Source files
SRCS = $(wildcard *.cc)

# Object files
OBJS = $(SRCS:.cc=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.cc.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
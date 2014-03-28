PROJECT = megasim

## Sources

SOURCES = megasim.c \
		  core.c \
		  decoder.c \
		  handlers.c 

## FLAGS
FLAGS = -g
CCFLAGS = $(FLAGS)
CXXFLAGS =

## Object Files
OBJS = $(SOURCES:.c=.c.o)

all: $(OBJS)
	$(CC) $(CCFLAGS) $^ -o $(PROJECT)
	
%.c.o : %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

%.cpp.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf $(PROJECT) $(OBJS) *.dSYM

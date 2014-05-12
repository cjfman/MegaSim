PROJECT = megasim

## Sources

SOURCES = megasim.c \
		  core.c \
		  decoder.c \
		  handlers.c \
		  debugterm.c \
		  devices.c \
		  opcode_defs.c \
		  peripherals.c \
		  debugterm.tab.c \
		  lex.yy.c

## FLAGS
DEFINES = -D NO_HARDWARE
		 #-D NO_TIMERS
		 #-D DEBUG_PERPH
		 #-D NO_PERPHS
	     #-D DEBUG
		 #-D DEBUG_CORE

FLAGS = -g -O0 $(DEFINES)
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

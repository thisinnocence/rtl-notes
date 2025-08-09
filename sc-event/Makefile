# Makefile for SystemC event demo programs

SRCS    = $(wildcard *.cc)
TARGETS = $(SRCS:.cc=.out)

.PHONY: all
all: $(TARGETS)

SYSTEMC_HOME = $(shell echo $$HOME)/github/systemc

INC_DIRS += $(SYSTEMC_HOME)/include
INC_DIRS += $(SYSTEMC_HOME)/examples/sysc/async_suspend

INC_FLAGS = $(addprefix -I, $(INC_DIRS))
CXXFLAGS  = $(INC_FLAGS) -MMD -MP -std=c++17 -Wall -g

LDFLAGS  = -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc
LDFLAGS += -Wl,-rpath,$(SYSTEMC_HOME)/lib-linux64
LDFLAGS += -lpthread

OBJS    = $(SRCS:.cc=.o)
DEPS    = $(SRCS:.cc=.d)
-include $(DEPS)

%.out: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.d *.out

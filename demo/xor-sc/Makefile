SRCS    = $(wildcard *.cc)
TARGET  = xor.bin

.PHONY: all
all: $(TARGET)

SYSTEMC_HOME = $(shell echo $$HOME)/github/systemc

INC_DIRS += $(SYSTEMC_HOME)/include

INC_FLAGS = $(addprefix -I, $(INC_DIRS))
CXXFLAGS  = $(INC_FLAGS) -MMD -MP -std=c++17 -Wall -g

LDFLAGS  = -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc
LDFLAGS += -Wl,-rpath,$(SYSTEMC_HOME)/lib-linux64
LDFLAGS += -lpthread

OBJS    = $(SRCS:.cc=.o)
DEPS    = $(SRCS:.cc=.d)
-include $(DEPS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.d $(TARGET)

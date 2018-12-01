CXX := g++
LD := $(CXX)
CPPFLAGS := -std=c++14 -g -Wall
#CPPFLAGS := -std=c++14 -O3 -Wall
LDLIBS := -lpthread -lrt -libverbs -lm

RMQ_HOME := ../src

OBJS := $(RMQ_HOME)/main.o $(RMQ_HOME)/mbuf.o $(RMQ_HOME)/transport.o \
	$(RMQ_HOME)/producer.o $(RMQ_HOME)/broker.o $(RMQ_HOME)/consumer.o

APPS := main

all: $(APPS)

main: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) 

.cpp.o:
	$(CXX) -c $< -o $@

clean:
	rm -f *.o $(APPS) $(OBJS)

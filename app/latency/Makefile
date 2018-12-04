CXX := g++
LD := $(CXX)
RMQ_SRC := ../../src
#CPPFLAGS := -std=c++14 -g -Wall -I$(RMQ_SRC)
CPPFLAGS := -std=c++14 -O3 -Wall -I$(RMQ_SRC)
LDLIBS := -lpthread -lrt -libverbs -lm

OBJS := main.o $(RMQ_SRC)/mbuf.o $(RMQ_SRC)/transport.o \
	$(RMQ_SRC)/producer.o $(RMQ_SRC)/broker.o $(RMQ_SRC)/consumer.o

APPS := main

all: $(APPS)

main: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) 

.cpp.o:
	$(CXX) -c $< -o $@

clean:
	rm -f *.o $(APPS) $(OBJS)

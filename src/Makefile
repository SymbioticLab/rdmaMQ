CXX := g++
LD := $(CXX)
CPPFLAGS := -std=c++14 -g -Wall
#CPPFLAGS := -std=c++14 -O3 -Wall
LDLIBS := -lpthread -lrt -libverbs -lm
OBJS := mbuf.o transport.o producer.o broker.o consumer.o main.o 

APPS := main

all: $(APPS)

main: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) 

.cpp.o:
	$(CXX) -c $< -o $@

clean:
	rm -f *.o $(APPS)

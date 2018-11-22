CC = g++ -std=c++11
CPPFLAGS = -g -Wall
LDLIBS = -lpthread -lrt -libverbs -lm
OBJS = main.o mbuf.o transport.o

APPS = test

all: $(APPS)

test: $(OBJS)
	$(CC) $(LDFLAGS) -o test $(OBJS)

main.o:
	$(CC) $(CPPFLAGS) -c main.cc

mbuf.o:
	$(CC) $(CPPFLAGS) -c mbuf.cc

transport.o:
	$(CC) $(CPPFLAGS) -c transport.cc

clean:
	rm -f *.o ${APPS}

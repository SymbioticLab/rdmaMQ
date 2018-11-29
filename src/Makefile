CC = g++ -std=c++14
CPPFLAGS = -g -Wall
LDLIBS = -lpthread -lrt -libverbs -lm
OBJS = main.o mbuf.o transport.o producer.o

APPS = test

all: $(APPS)

test: $(OBJS)
	$(CC) -o test $(OBJS) $(LDLIBS) 

main.o:
	$(CC) $(CPPFLAGS) -c main.cc

mbuf.o:
	$(CC) $(CPPFLAGS) -c mbuf.cc

transport.o:
	$(CC) $(CPPFLAGS) -c transport.cc

producer.o:
	$(CC) $(CPPFLAGS) -c producer.cc

clean:
	rm -f *.o ${APPS}

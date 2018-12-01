CC = g++ -std=c++14
CPPFLAGS = -g -Wall
LDLIBS = -lpthread -lrt -libverbs -lm
OBJS = main.o mbuf.o transport.o producer.o broker.o consumer.o

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

broker.o:
	$(CC) $(CPPFLAGS) -c broker.cc

consumer.o:
	$(CC) $(CPPFLAGS) -c consumer.cc

clean:
	rm -f *.o ${APPS}

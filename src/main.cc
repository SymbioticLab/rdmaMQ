#include <iostream>
#include "rmq.h"
#include "mbuf.h"
#include "transport.h"
#include "producer.h"

int main() {
	//auto transport = std::make_shared<rmq::Transport>();
	//rmq::MessageBuffer<int> buffer(10, transport->get_pd());
	auto producer = new rmq::Producer<int>(1000, "10.0.0.2");
	std::cout << "hello world" << std::endl;

	return 0;
}
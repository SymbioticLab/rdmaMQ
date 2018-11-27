#include <iostream>
#include "rmq.h"
#include "mbuf.h"
#include "transport.h"

int main() {
	auto transport = std::make_shared<rmq::Transport>();
	rmq::MessageBuffer<int> buffer(10, transport->get_pd());
	std::cout << "hello world" << std::endl;

	return 0;
}
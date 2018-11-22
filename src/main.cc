#include <iostream>
#include "rmq.h"
#include "mbuf.h"
#include "transport.h"

int main() {
	rmq::MessageBuffer<int> buffer(10);
	std::cout << "hello world" << std::endl;

	return 0;
}
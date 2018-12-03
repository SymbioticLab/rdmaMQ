#include <iostream>
#include "rmq.h"
#include "mbuf.h"
#include "transport.h"
#include "producer.h"
#include "broker.h"
#include "consumer.h"

void run_producer() {
    std::cout << "Running as Producer" << std::endl;
    auto producer = new rmq::Producer<int>(1000, "10.0.0.2");
    producer->init_transport();
    /*
    for (int i = 0; i < 100; i++) {
        producer->data()[i] = i;
        producer->push(i, 1);
        //std::cout << "Data: " << producer->data()[i] << std::endl;
    }
    */
    while(1) {}
}

void run_consumer() {
    std::cout << "Running as Consumer" << std::endl;
    auto consumer = new rmq::Consumer<int>(1000, "10.0.0.2");
    consumer->init_transport();
    consumer->fetch_write_idx();
    for (int i = 0; i < 100; i++) {
        consumer->pull(i, i, 1);
        std::cout << "Data read: " << consumer->data()[i] << std::endl;
    }
    while(1) {}
}

void run_broker() {
    std::cout << "Running as Broker" << std::endl;
    auto broker = new rmq::Broker<int>();
    broker->init_transport();
    while (1) {
        //std::cout << broker->ctrl()[0] << std::endl;
    }        // can't return
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: ./main ROLE [BROKER_IP]" << std::endl;
        std::cout << "ROLE -- 0: Producer; 1: Consumer; 2: Broker" << std::endl;
        std::cout << "A Producer or Consumer must specify Broker IP" << std::endl;
        exit(1);
    }
    char* end_ptr;
    // ROLE -- 0: Producer; 1: Consumer; 2: Broker
    rmq::Role role = static_cast<rmq::Role>(strtol(argv[1], &end_ptr, 10));

    if (role == rmq::Role::rmq_Producer) {
        if (argc != 3) {
            std::cout << "Usage: ./main ROLE [BROKER_IP]" << std::endl;
            exit(1);
        }
        run_producer();
    } else if (role == rmq::Role::rmq_Consumer) {
        if (argc != 3) {
            std::cout << "Usage: ./main ROLE [BROKER_IP]" << std::endl;
            exit(1);
        }
        run_consumer();
    } else if (role == rmq::Role::rmq_Broker) {
        run_broker();
    } else {
        std::cout << "Invalid role. Exiting." << std::endl;
        exit(1);
    }

    std::cout << "Jump out of loop. Shouldn't see this." << std::endl;

    //auto transport = std::make_shared<rmq::Transport>();
    //rmq::MessageBuffer<int> buffer(10, transport->get_pd());

    return 0;
}

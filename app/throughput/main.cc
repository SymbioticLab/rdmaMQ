#include <iostream>
#include "rmq.h"
#include "mbuf.h"
#include "transport.h"
#include "producer.h"
#include "broker.h"
#include "consumer.h"
#include "measure.h"
#include <algorithm>
#include <math.h>

size_t NUM_REQ = 10000000;
size_t BATCH_SIZE = 500;
size_t start_cycle;
size_t end_cycle;

void report_perf(size_t duration) {
    double cpu_mhz = rmq::get_cpu_mhz();
    double total_time = (double)duration / cpu_mhz;
    double tput = (double)NUM_REQ / total_time * 1000000;
    printf("@MEASUREMENT:\n");
    printf("total time = %.2f\n", total_time);
    printf("THROUGHPUT = %.2f\n", tput);
}

void run_producer() {
    std::cout << "Running as Producer" << std::endl;
    auto producer = new rmq::Producer<int>(NUM_REQ, "10.0.0.2");
    producer->init_transport();
    start_cycle = rmq::get_cycles();
    for (size_t i = 0; i < NUM_REQ; i+=BATCH_SIZE) {
        //producer->push(0, BATCH_SIZE);
        producer->push_batch(0, BATCH_SIZE);
        //std::cout << "Data: " << producer->data()[i] << std::endl;
    }
    end_cycle = rmq::get_cycles();
    size_t duration = end_cycle - start_cycle;
    report_perf(duration);
    //while(1) {}
}

void run_consumer() {
    std::cout << "Running as Consumer" << std::endl;
    auto consumer = new rmq::Consumer<int>(NUM_REQ, "10.0.0.2");
    consumer->init_transport();
    std::cout << "remote write idx = " << consumer->fetch_write_idx() << std::endl;
    start_cycle = rmq::get_cycles();
    for (size_t i = 0; i < NUM_REQ; i+=BATCH_SIZE) {
        size_t read_idx = i % rmq::bkr_buff_cap;
        //consumer->pull(0, read_idx, BATCH_SIZE);
        consumer->pull_batch(0, read_idx, BATCH_SIZE);
        //std::cout << "Data read: " << consumer->data()[i] << std::endl;
    }
    end_cycle = rmq::get_cycles();
    size_t duration = end_cycle - start_cycle;
    report_perf(duration);
    //while(1) {}
}

void run_broker() {
    std::cout << "Running as Broker" << std::endl;
    auto broker = new rmq::Broker<int>(2);
    broker->init_transport();
    while (1) {
        sleep(10);
        //std::cout << broker->ctrl()[0] << std::endl;
    } // can't return
}

int main(int argc, char **argv) {
    std::string usage = std::string("Usage: ./main ROLE [BROKER_IP]\n") + 
                        std::string("ROLE -- 0:Producer // 1:Consumer // 2:Broker\n") +
                        std::string("A Producer or Consumer must specify Broker IP\n");
    if (argc < 2) {
        std::cout << usage << std::endl;
        exit(1);
    }
    char* end_ptr;
    // ROLE -- 0: Producer; 1: Consumer; 2: Broker
    rmq::Role role = static_cast<rmq::Role>(strtol(argv[1], &end_ptr, 10));

    if (role == rmq::Role::rmq_Producer) {
        if (argc != 3) {
            std::cout << usage << std::endl;
            exit(1);
        }
        run_producer();
    } else if (role == rmq::Role::rmq_Consumer) {
        if (argc != 3) {
            std::cout << usage << std::endl;
            exit(1);
        }
        run_consumer();
    } else if (role == rmq::Role::rmq_Broker) {
        run_broker();
    } else {
        std::cout << "Invalid role. Exiting." << std::endl;
        exit(1);
    }

    //std::cout << "Jump out of loop. Shouldn't see this." << std::endl;

    //auto transport = std::make_shared<rmq::Transport>();
    //rmq::MessageBuffer<int> buffer(10, transport->get_pd());

    return 0;
}

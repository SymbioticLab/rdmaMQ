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

size_t NUM_REQ = 1000000;
size_t *start_cycles;
size_t *end_cycles;

void init_lat_array() {
    start_cycles = new size_t[NUM_REQ];
    end_cycles = new size_t[NUM_REQ];
}

void report_perf() {
    auto lat = new double[NUM_REQ];
    double cpu_mhz = rmq::get_cpu_mhz();
    for (size_t i = 0; i < NUM_REQ; i++) {
        lat[i] = (double)(end_cycles[i] - start_cycles[i]) / cpu_mhz;
    }
    std::sort(lat, lat + NUM_REQ);
    size_t idx_m, idx_99, idx_99_9, idx_99_99;
    size_t measure_cnt = NUM_REQ;
    idx_m = ceil(measure_cnt * 0.5);
    idx_99 = ceil(measure_cnt * 0.99);
    idx_99_9 = ceil(measure_cnt * 0.999);
    idx_99_99 = ceil(measure_cnt * 0.9999);
    //printf("idx_99 = %lu; idx_99_t = %lu; idx_99_99 = %lu\n", idx_99, idx_99_9, idx_99_99);
    printf("@MEASUREMENT:\n");
    printf("MEDIAN = %.2f\n", lat[idx_m]);
    printf("99 TAIL = %.2f\n", lat[idx_99]);
    printf("99.9 TAIL = %.2f\n", lat[idx_99_9]);
    printf("99.99 TAIL = %.2f\n", lat[idx_99_99]);
}

void run_producer() {
    std::cout << "Running as Producer" << std::endl;
    init_lat_array();
    auto producer = new rmq::Producer<int>(NUM_REQ, "10.0.0.2");
    producer->init_transport();
    for (size_t i = 0; i < NUM_REQ; i++) {
        producer->data()[i] = i;
        start_cycles[i] = rmq::get_cycles();
        producer->push(i, 1);
        end_cycles[i] = rmq::get_cycles();
        //std::cout << "Data: " << producer->data()[i] << std::endl;
    }
    report_perf();
    //while(1) {}
}

void run_consumer() {
    std::cout << "Running as Consumer" << std::endl;
    init_lat_array();
    auto consumer = new rmq::Consumer<int>(NUM_REQ, "10.0.0.2");
    consumer->init_transport();
    std::cout << "remote write idx = " << consumer->fetch_write_idx() << std::endl;
    for (size_t i = 0; i < NUM_REQ; i++) {
        start_cycles[i] = rmq::get_cycles();
        size_t read_idx = i % rmq::bkr_buff_cap;
        consumer->pull(0, read_idx, 1);
        end_cycles[i] = rmq::get_cycles();
        //std::cout << "Data read: " << consumer->data()[i] << std::endl;
    }
    report_perf();
    //while(1) {}
}

void run_broker() {
    std::cout << "Running as Broker" << std::endl;
    auto broker = new rmq::Broker<int>(2);
    broker->init_transport();
    while (1) {
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

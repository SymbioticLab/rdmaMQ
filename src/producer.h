#ifndef PRODUCER_H_
#define PRODUCER_H_

#include "mbuf.h"
#include "transport.h"

namespace rmq {

/**
 * Producers create MessageBuffers and push their content to
 * the broker. A Producer picks which idx and length of the
 * buffer to push. Connection setup is done after each buffer
 * is created. There is another mr for control msg (e.g. remote
 * write addr) used with atomic ib verbs
 * 
 * For simplicity, each Producer only creates one buffer instead
 * of multiple with different types.
 * 
 * In the aspect of tranport, a Producer is a sender rather than
 * a receiver. Users will pass in the ip of the receiver (in this
 * case the broker) to set up RDMA connection.
 * 
 * Users need to manually call init_tranport() after constructing Producer.
 * 
 * Currently we don't support one Producer pushing to multiple topics
 * or brokers. Which broker to push to is decided when constructing Producer.
 * 
 * Usage:
 *      First call Producer custom constructor.
 *      Then construct data_buf and ctrl_buf.
 *      Then call Producer::init_transport() to complete the setup.
 */

template <typename T>
class Producer {
private:
    std::unique_ptr<MessageBuffer<T>> data_buf;             // assume one buffer for now
    std::unique_ptr<MessageBuffer<uint64_t>> ctrl_buf;      // store write_addr
    std::unique_ptr<Transport> transport;
    std::string broker_ip;

    size_t fetch_and_add_write_addr(size_t start_idx, size_t num_msg);

public:
    Producer() {}
    Producer(size_t data_buf_cap, std::string broker_ip)
    : broker_ip(std::string(broker_ip)) {
        transport = std::make_unique<Transport>();
        data_buf = std::make_unique<MessageBuffer<T>>(data_buf_cap, transport->get_pd());
        ctrl_buf = std::make_unique<MessageBuffer<uint64_t>>(1, transport->get_pd(), 1);
    }
    ~Producer() {}

    // gets called after constrcut mbuf
    void init_transport(int gid_idx) {
        transport->init(broker_ip.c_str(), data_buf->get_mr(), ctrl_buf->get_mr(), gid_idx);
    }

    // start_idx: indicates the starting address of the data in the buffer pushed to the broker
    // num_msg: # of data blocks to send in a batch
    // e.g., push(0, 2) pushes the first 2 elements in the buffer to the broker
    // returns num_msg actually written
    size_t push(size_t start_idx, size_t num_msg);

};





}


#endif // PRODUCER_H_
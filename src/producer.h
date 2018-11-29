#ifndef PRODUCER_H_
#define PRODUCER_H_

#include "mbuf.h"
#include "transport.h"

namespace rmq {

/**
 * Producer creates MessageBuffers and push their content to
 * the broker. A producer pick which idx and length of the
 * buffer to push. Connection setup is done after each buffer
 * is created. There is another mr for control msg (e.g. remote
 * write addr) used with atomic ib verbs
 * 
 * For simplicity, each producer only creates one buffer instead
 * of multiple with different types.
 * 
 * In the aspect of tranport, producer is a sender rather than
 * receiver. User will pass in the ip of the receiver (in this
 * case the broker) to set up RDMA connection.
 * 
 * User needs to manually call init_tranport() after constructing Producer.
 * 
 * Currently we don't support one producer pushing to multiple topics
 * or brokers.
 */

template <typename T>
class Producer {
private:
    std::unique_ptr<MessageBuffer<T>> data_buf;             // assume one buffer for now
    std::unique_ptr<MessageBuffer<uint64_t>> ctrl_buf;      // store write_addr
    std::unique_ptr<Transport> transport;
    std::string broker_ip;

public:
    Producer() {}
    Producer(size_t data_buf_cap, std::string broker_ip)
    : broker_ip(std::string(broker_ip)) {
        transport = std::make_unique<Transport>();
        data_buf = std::make_unique<MessageBuffer<T>>(data_buf_cap, transport->get_pd());
        ctrl_buf = std::make_unique<MessageBuffer<uint64_t>>(1, transport->get_pd(), 1);
    }
    ~Producer() {}

    void init_transport(int gid_idx) {
        transport->init(broker_ip.c_str(), data_buf->get_mr(), ctrl_buf->get_mr(), gid_idx);
    }

    int fetch_and_add_write_addr();

    // assume only one databuf sends to only one broker
    // num_msg = # of data blocks to send in a batch
    int push(size_t num_msg);

};





}


#endif // PRODUCER_H_
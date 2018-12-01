#ifndef BROKER_H_
#define BROKER_H_

#include "mbuf.h"
#include "transport.h"

namespace rmq {

/**
 * Brokers create MessageBuffers and stores the content Producers
 * push so that Consumers can read from it. 
 * 
 * Buffer sizes are known to the Producers and Consumers (through
 * global configuration) for a simple implementation. Brokers act 
 * as servers when initializing connections with Producers and Consumers.
 * 
 * In the current design, Brokers are completely passive, meaning
 * no CPU usage at all. This is based on the assumption that Producers
 * can overwrite the data that haven't been read by Consumers. In such
 * case, Brokers do not need to keep track of what has been read or not,
 * thus no CPU consumption (together with the help of one-sided RDMA verbs).
 * 
 * 
 * Usage:
 *      Broker needs to connect with producer. and consumers.
 *      First call Broker custom constructor.
 *      Then call Broker::init_transport() to complete the setup.
 *      Then construct Producer and call Producer::init_transport().
 *      Then construct Consumer and call Consumer::init_transport().
 * 
 *      TODO: better cluster initalization mechanism.
 */

template <typename T>
class Broker {
private:
    // For broker, mbuf is not unique (shared by at producers and consumers)
    MessageBuffer<T> *data_buf;             // assume one buffer for now
    MessageBuffer<uint64_t> *ctrl_buf;      // store write_addr
    std::unique_ptr<Transport> transport;

public:
    Broker() {
        transport = std::make_unique<Transport>();
        data_buf = new MessageBuffer<T>(bkr_buff_cap, transport->get_pd());
        ctrl_buf = new MessageBuffer<uint64_t>(1, transport->get_pd(), 1);
    }
    ~Broker() {
        delete data_buf;
        delete ctrl_buf;
    }

    // gets called after constructing mbuf
    void init_transport(int gid_idx = -1) {
        // initialize ctrl_buf (which contains loop_cnt and write_idx)
        memset(ctrl_buf->get_data(), 0, ctrl_buf->get_block_size());

        transport->init(data_buf->get_mr(), 2, ctrl_buf->get_mr(), gid_idx);
    }

};




}


#endif // BROKER_H_
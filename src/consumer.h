#ifndef CONSUMER_H_
#define CONSUMER_H_

#include "mbuf.h"
#include "transport.h"

namespace rmq {

/**
 * Consumers create MessageBuffers and read topic they subscribed 
 * from the broker. Consumers first read the available offset from
 * the broker before issue a read. It can decide when and how often
 * to fetch the offset, and currently there is no guarantee the data
 * get overwritten by Producers before Consuers read them since we
 * pick space retention mechanism instead of time retention mechanism
 * used by Kafka.
 * 
 * Like Producers, each Consumer creates one data buffer and one
 * control buffer. The control buffer is for storing the offset.
 * 
 * In the aspect of tranport, a Consumer is a sender rather than
 * a receiver. Users will pass in the ip of the receiver (in this
 * case the broker) to set up RDMA connection.
 * 
 * Users need to manually call init_tranport() after constructing Consumer.
 * 
 * Currently we assume each Consumer only subscribe to one topic,
 * and thus there is no need to do the subscription process. (/huaji)
 * 
 * Usage:
 *      First call Consumer custom constructor,
 *      which constructs data_buf and ctrl_buf.
 *      Then call Consumer::init_transport() to complete the setup.
 */

template <typename T>
class Consumer {
private:
    std::unique_ptr<MessageBuffer<T>> data_buf;             // assume one buffer for now
    std::unique_ptr<MessageBuffer<uint64_t>> ctrl_buf;      // store write_addr
    std::unique_ptr<Transport> transport;
    std::string broker_ip;

public:
    Consumer() {}
    Consumer(size_t data_buf_cap, std::string broker_ip)
    : broker_ip(std::string(broker_ip)) {
        transport = std::make_unique<Transport>();
        data_buf = std::make_unique<MessageBuffer<T>>(data_buf_cap, transport->get_pd());
        ctrl_buf = std::make_unique<MessageBuffer<uint64_t>>(1, transport->get_pd(), 1);
    }
    ~Consumer() {
        LOG_DEBUG("Broker destructor gets called\n");
    }

    // gets called after constructing mbuf
    void init_transport(int gid_idx = -1) {
        transport->init(broker_ip.c_str(), 1, data_buf->get_mr(), ctrl_buf->get_mr(), gid_idx);
    }

    // assume only one topic
    // TODO: move this to public and return idx to users
    void fetch_write_idx();

    // start_idx: indicates the starting address of the data in the buffer pulled from the broker
    // read idx: indicates where in the remote broker buffer will the read start from
    // num_msg: # of data blocks to read in a batch
    // e.g., pull(0, 0, 2) pushes the first 2 elements in the broker buffer to the first 2 elements
    // of the local buffer
    // returns num_msg actually read
    size_t pull(size_t start_idx, size_t read_idx, size_t num_msg);

    inline T *data() {
        return data_buf->get_data();
    }

    inline uint64_t *ctrl() {
        return ctrl_buf->get_data();
    }

};





}


#endif // CONSUMER_H_
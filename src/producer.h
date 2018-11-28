#ifndef PRODUCER_H_
#define PRODUCER_H_

#include "mbuf.h"

namespace rmq {

/**
 * Producer creates MessageBuffers and push their content to
 * the broker. A producer pick which idx and length of the
 * buffer to push. Connection setup is done after each buffer
 * is created.
 * 
 * For simplicity, each producer only creates one buffer instead
 * of multiple with different types.
 * 
 * In the aspect of tranport, producer is a sender rather than
 * receiver. User will pass in the ip of the receiver (in this
 * case the broker) to set up RDMA connection.
 * 
 * User needs to manually call init() after constructing Producer.
 */

template <typename T>
class Producer {
private:
    std::unique<MessageBuffer<int>> mbuf;        // assume type int for now
    std::unique_ptr<Transport> transport;
    std::string broker_ip;


public:
    Producer() {}
    Producer(size_t mbuf_cap, char *broker_ip)
    : broker_ip(std::string(broker_ip)) {
        transport = std::make_unique<Transport>();
        mbuf = std::make_unique<MessageBuffer<int>> MessageBuffer(mbuf_cap, transport->get_pd());
    }
    ~Producer() {}

    void init_transport(int gid_idx) {
        transport.init(broker_ip.c_str(), mbuf->get_rkey(), 
            std::static_cast<uintptr_t>(mbuf->get_data()), gid_idx);
    }

}





}


#endif // PRODUCER_H_
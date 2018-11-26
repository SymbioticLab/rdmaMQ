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
 */

template <typename T>
class Producer {
private:
    // Basic of the basic
    MessageBuffer<int> mbuf;        // type int for simplicity

    // RDMA transport metadata
    std::shared_ptr<Transport> transport;

    // Broker info (brokerIP, raddr, etc)
    std::string broker_ip;

    


public:



}





}


#endif // PRODUCER_H_
#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <infiniband/verbs.h>
#include "rmq.h"

namespace rmq {

/**
 * Handles RDMA context/connection setup before MessageBuffer 
 * registers memory region and Producer and Consumer transfer data.
 * Currently, the class is instantiated with the default constr.
 */

class Transport {
private:
    struct ibv_context *ctx;
    struct ibv_pd *pd;

    // Open ib device and alloc pd.
    void init();
    
public:
    Transport() { init(); }
    ~Transport() {}
    inline struct ibv_context ibv_get_ctx() { return ctx; }
    inline struct ibv_pd get_pd() { return pd; }
    //inline struct ibv_mr get_mr() { return mr; }
    //inline void set_mr(struct ibv_mr *mr) { this->mr = mr; }
};




}

#endif // TRANSPORT_H_
#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <infiniband/verbs.h>
#include "common.h"
//#include "rmq.h"

namespace rmq {

//template<typename T>
//class MessageBuffer;

/**
 * Handles RDMA context/connection setup before MessageBuffer 
 * registers memory region and Producer and Consumer transfer data.
 * Currently, the class is instantiated with the default constr.
 */

class Transport {
//template<typename T>
//friend class MessageBuffer<T>;
private:
    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;

    void open_device();
    void alloc_pd();
    void init();        // wrapper of open_device() and alloc_pd()
    void create_cq();
    void create_qp();

public:
    Transport() { init(); }
    ~Transport() {}
    inline struct ibv_context *ibv_get_ctx() { return ctx; }
    inline struct ibv_pd *get_pd() { return pd; }
    //inline struct ibv_mr get_mr() { return mr; }
    //inline void set_mr(struct ibv_mr *mr) { this->mr = mr; }
};




}

#endif // TRANSPORT_H_
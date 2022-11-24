#pragma once
#include "Commons.h"
#include "MsgHandler.h"

/// @brief Lock interface
class Lock {
public:
    virtual ~Lock() = default;

    virtual void requestCS() = 0;    
    virtual void releaseCS() = 0;
protected:

    /* Mutex only used to make thread wait */
    std::mutex mtx_Wait;
    std::condition_variable cv_Wait;

};
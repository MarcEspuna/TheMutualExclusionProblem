#pragma once
#include "Commons.h"
#include "MsgHandler.h"

/// @brief Lock interface
class Lock {
public:
    Lock() = default;
    virtual ~Lock() = default;

    virtual void requestCS() = 0;    
    virtual void releaseCS() = 0;

};
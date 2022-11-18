#include "Commons.h"
#include "MsgHandler.h"

/// @brief Lock interface
class Lock {
public:
    virtual void requestCS() = 0;    
    virtual void releaseCS() = 0;
};
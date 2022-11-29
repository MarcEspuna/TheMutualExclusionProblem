#pragma once
#include "Commons.h"
#include "MsgHandler.h"



/// @brief Lock interface
class Lock : public MsgHandler {
public:
    Lock(const Linker& link) : MsgHandler(link) {};
    virtual ~Lock() = default;

    virtual void requestCS() = 0;    
    virtual void releaseCS() = 0;

    template<typename T>
    static Lock* Create(const Linker& link) { return new T(link); }

    template<typename T>
    static Lock* Create(const Linker& link, bool leader) { return new T(link, leader); }

protected:
    /* Mutex only used to make thread wait */
    std::mutex mtx_Wait;
    std::condition_variable cv_Wait;   
};
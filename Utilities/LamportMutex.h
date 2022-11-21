#pragma once
#include "Commons.h"
#include "MsgHandler.h"
#include "Lock.h"
#include "Log.h"
#include "DirectClock.h"

class LamportMutex : public MsgHandler, Lock {
public:
    LamportMutex(const Linker& link);
    ~LamportMutex();

    void requestCS() override;
    void releaseCS() override;

    void HandleMsg(int message, int src, Tag tag) override;          /* Used for current level processes */
    void HandleChildMsg(int message, int src, Tag tag) override;     /* Used for child porcesses */
private:
    bool okeyCS();
    
private:
    int m_Id;
    DirectClock m_Clock;
    std::priority_queue<std::pair<int, int>> m_RequestQ;             // <<ticks><Id>>
    
    /* Mutex only used to make thread wait */
    std::mutex mtx_Wait;
    std::condition_variable cv_Wait;   
};

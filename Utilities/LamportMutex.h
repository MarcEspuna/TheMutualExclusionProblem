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
    using Priority_queue = std::priority_queue<std::pair<int, int>, std::vector<std::pair<int,int>>, std::greater<std::pair<int,int>>>;

    DirectClock m_Clock;
    Priority_queue m_RequestQ;             // <<ticks><Id>>
    int m_ConnReadyCount;
    
};

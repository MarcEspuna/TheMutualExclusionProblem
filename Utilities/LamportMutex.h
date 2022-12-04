#pragma once
#include "Commons.h"
#include "MsgHandler.h"
#include "Lock.h"
#include "Log.h"
#include "DirectClock.h"


class LamportMutex : public Lock {
public:
    LamportMutex(const Linker& link);
    virtual ~LamportMutex();

    void requestCS() override;
    void releaseCS() override;

    void HandleMsg(int message, int src, Tag tag) override;          /* Used for current level processes */
    void HandleChildMsg(int message, int src, Tag tag) override;     /* Used for child porcesses */

private:
    bool okeyCS();
    
private:
    DirectClock m_Clock;
    std::unordered_map<int, int> m_RequestQ;

    std::mutex mtx_RequestQ;
};

#pragma once
#include "Commons.h"
#include "Lock.h"
#include "MsgHandler.h"
#include "LamportClock.h"

class RAMutex : public MsgHandler, Lock{
public:
    RAMutex(const Linker& link);
    ~RAMutex();

    void requestCS() override;
    void releaseCS() override;

    void HandleMsg(int message, int src, Tag tag) override;          /* Used for current level processes */
    void HandleChildMsg(int message, int src, Tag tag) override;     /* Used for child porcesses */

private:
    bool okeyCS();

private:
    int m_Myts;
    int m_NumOkey;
    LamportClock m_Clock;
    std::queue<int> m_PendingQ;
};


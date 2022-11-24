#pragma once
#include "Commons.h"
#include "Socket.h"
#include "Lock.h"
#include "MsgHandler.h"
#include <queue>

class CentMutex : public Lock, MsgHandler {
public:
    CentMutex(const Linker& coms, bool leader);
    virtual ~CentMutex();

    void requestCS() override;    
    void releaseCS() override;

    void HandleMsg(int message, int src, Tag tag) override;
    void HandleChildMsg(int message, int src, Tag tag) override;

private:
    bool m_Leader;
    bool m_Token;

    std::queue<int> pendingQ;

    int m_ChildFinishes;

};
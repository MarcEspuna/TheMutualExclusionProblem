#include "CentMutex.h"
#include "HeavyWeights/HWApp.h"
#include "Log.h"
#include "App.h"

CentMutex::CentMutex(const Linker& coms, bool leader)
    : Lock(coms), m_Leader(leader), m_Token(leader), m_ChildFinishes(0) 
{
    LOG_INFO("Centralized mutex created");
}

CentMutex::~CentMutex()
{
    
}


void CentMutex::requestCS()
{
    LOG_TRACE("Requesting token to leader\n");
    LOG_TRACE("Sending request.\n");
    App::SendMsg(m_ParentId,Tag::REQUEST);
    LOG_TRACE("Waiting for token\n");
    std::unique_lock<std::mutex> lk(mtx_Wait);
    cv_Wait.wait(lk, [&](){return m_Token;});
    LOG_TRACE("Token received!\n");
}    

void CentMutex::releaseCS()
{
    m_Token = false;
    App::SendMsg(m_ParentId, Tag::RELEASE);
}


void CentMutex::HandleMsg(int message, int src, Tag tag)
{
    LOG_INFO("Received, tag: {}, message: {}\n", (char)tag, message);
    switch (tag)
    {
    /* CENTRALIZED MUTEX */
    case Tag::REQUEST:
        if (m_Token)
        {
            LOG_TRACE("LEADER, Handling token to child process.\n");
            App::SendMsg(src, Tag::OK);
            m_Token = false;
        } else {
            LOG_TRACE("LEADER, Adding to the queue.\n");
            pendingQ.push(src);
        }
        break;
    case Tag::RELEASE:
        LOG_TRACE("LEADER, Release receved.\n");
        if (!pendingQ.empty())
        {
            LOG_TRACE("LEADER, Giving token to the pending queue.\n");
            App::SendMsg(pendingQ.front(), Tag::OK);
            pendingQ.pop();
        } else  {
            m_Token = true;
            LOG_TRACE("LEADER, Got token\n");
        } 
        break;
    case Tag::OK:
        LOG_TRACE("CHILD, Token received.\n");
        m_Token = true;
        cv_Wait.notify_all();
        break;
    }


}


void CentMutex::HandleChildMsg(int message, int src, Tag tag)
{
    HandleMsg(message, src, tag);
}
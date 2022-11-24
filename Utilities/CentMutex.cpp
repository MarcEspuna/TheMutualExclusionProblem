#include "CentMutex.h"
#include "Log.h"

CentMutex::CentMutex(const Linker& coms, bool leader)
    : MsgHandler(coms), m_Leader(leader), m_Token(false), m_ChildFinishes(0)
{
    if (leader)     m_Token = true;
    // Wait for connections to finish
}

CentMutex::~CentMutex()
{
    
}

void CentMutex::requestCS()
{
    SendMsgParent(Tag::REQUEST);
    std::unique_lock<std::mutex> lk(mtx_Wait);
    cv_Wait.wait(lk, [&](){return m_Token;});
}    

void CentMutex::releaseCS()
{
    m_Token = false;
    SendMsgParent(Tag::RELEASE);
}


void CentMutex::HandleMsg(int message, int src, Tag tag)
{
    LOG_INFO("Received, tag: {}, message: {}\n", (char)tag, message);
    switch (tag)
    {
    case Tag::REQUEST:
        if (m_Token)
        {
            LOG_TRACE("LEADER, Handling token to child process.\n");
            SendMsg(src, Tag::OK);
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
            SendMsg(pendingQ.front(), Tag::OK);
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
    case Tag::ACK:
        LOG_TRACE("CHILD, child process finished\n");
        m_ChildFinishes++;
        break;
    default:
        break;
    }
}


void CentMutex::HandleChildMsg(int message, int src, Tag tag)
{
    HandleMsg(message, src, tag);
}
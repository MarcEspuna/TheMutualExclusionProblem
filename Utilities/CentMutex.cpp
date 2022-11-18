#include "CentMutex.h"

CentMutex::CentMutex(const Linker& coms, bool leader)
    : MsgHandler(coms), m_Leader(leader), m_Token(false)
{
    if (leader)     m_Token = true;
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
    SendMsgParent(Tag::RELEASE);
    m_Token = false;
}


void CentMutex::HandleMsg(int message, Socket* src, Tag tag)
{
    switch (tag)
    {
    case Tag::REQUEST:
        if (m_Token)
        {
            SendMsg(src, Tag::OK);
            m_Token = false;
        } else 
            pendingQ.push(src);
        break;
    case Tag::RELEASE:
        if (!pendingQ.empty())
        {
            Socket* next = pendingQ.front();
            SendMsg(next, Tag::OK);
            pendingQ.pop();
        } else  m_Token = true; 
        break;
    case Tag::OK:
        m_Token = true;
        cv_Wait.notify_all();
        break;
    default:
        break;
    }
}


void CentMutex::HandleChildMsg(int message, Socket* src, Tag tag)
{
    HandleMsg(message, src, tag);
}
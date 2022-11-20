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
    std::cout << "Receved, tag: " << (char)tag  << " message: " << message << std::endl;
    switch (tag)
    {
    case Tag::REQUEST:
        if (m_Token)
        {
            std::cout << "[LEADER]: Handling token to child process.\n";
            SendMsg(src, Tag::OK);
            m_Token = false;
        } else {
            std::cout << "[LEADER]: Adding to the queue\n";
            pendingQ.push(src);
        }
        break;
    case Tag::RELEASE:
        std::cout << "[LEADER]: Release receved.\n";
        if (!pendingQ.empty())
        {
            std::cout << "[LEADER]: Giving token to the pending queue.\n";
            Socket* next = pendingQ.front();
            SendMsg(next, Tag::OK);
            pendingQ.pop();
        } else  {
            m_Token = true;
            std::cout << "[LEADER]: Got token\n";
        } 
        break;
    case Tag::OK:
        std::cout << "[CHILD PROCESS]: Token received.\n";
        m_Token = true;
        cv_Wait.notify_all();
        break;
    default:
        break;
    }
}


void CentMutex::HandleChildMsg(int message, Socket* src, Tag tag)
{
    std::cout << "Child message\n" << std::endl;
    HandleMsg(message, src, tag);
}
#include "RAMutex.h"
#undef max

static bool interestedCS(int otherTicks, int myTicks, int otherID, int myId);

RAMutex::RAMutex(const Linker& link)
    : MsgHandler(link), m_Myts(std::numeric_limits<int>::max()) 
{


}

RAMutex::~RAMutex()
{

}


void RAMutex::requestCS()
{
    m_Clock.Tick();
    m_Myts = m_Clock.GetValue();
    BroadcastMsg(Tag::REQUEST, m_Myts);
    m_NumOkey = 0;
    std::unique_lock<std::mutex> lck;
    cv_Wait.wait(lck, [&](){return m_NumOkey >= ConnectionSize()-1;});
}

void RAMutex::releaseCS()
{
    m_Myts = std::numeric_limits<int>::max();
    while (!m_PendingQ.empty())
    {
        int id = m_PendingQ.front();
        SendMsg(id, Tag::OK, m_Clock.GetValue());
        m_PendingQ.pop();
    }
}

void RAMutex::HandleMsg(int msg, int src, Tag tag)
{
    m_Clock.ReceiveAction(src, msg);
    switch (tag)
    {
    case Tag::REQUEST:
        if (interestedCS(msg, m_Myts, src, m_Id))   
            SendMsg(src, Tag::OK, m_Clock.GetValue());
        else                                        
            m_PendingQ.push(src);
        break;
    case Tag::OK:
        m_NumOkey++;
        cv_Wait.notify_all();
        break;
    default:
        break;
    }
}

void RAMutex::HandleChildMsg(int msg, int src, Tag tag)
{
    HandleMsg(msg, src, tag);
}

static bool interestedCS(int timestamp, int myts, int otherID, int myId)
{
    return myts == std::numeric_limits<int>::max()   // Not interested in CS
            ||  (timestamp < myts) 
            ||  (timestamp == myts && otherID < myId);
}
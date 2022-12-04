#include "RAMutex.h"
#include "Log.h"
#include "App.h"
#undef max

static bool interestedCS(int otherTicks, int myTicks, int otherID, int myId);

RAMutex::RAMutex(const Linker& link)
    : Lock(link), m_NumOkey({0}), m_Myts(std::numeric_limits<int>::max()), m_Clock()
{
}

RAMutex::~RAMutex()
{
}


void RAMutex::requestCS()
{
    LOG_TRACE("Request CS.\n");
    m_Clock.Tick();
    m_Myts = m_Clock.GetValue();
    m_NumOkey = 0;                          // Important to put it to 0 BEFORE broadcasting request
    BroadcastMsg(Tag::REQUEST, m_Myts);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return m_NumOkey >= m_CurrentComms.size();});
    LOG_WARN("Entering mutex area.\n");
}

void RAMutex::releaseCS()
{
    LOG_INFO("Releasing CS.\n");
    m_Myts = std::numeric_limits<int>::max();
    while (!m_PendingQ.empty())
    {
        int id = m_PendingQ.front();
        App::SendMsg(id, Tag::OK, m_Clock.GetValue());
        m_PendingQ.pop();
    }
}

void RAMutex::HandleMsg(int msg, int src, Tag tag)
{
    LOG_TRACE("Msg received, tag, {}, ticks {}, my id {}, src id {}.\n", (char)tag, msg, m_Id, src);
    m_Clock.ReceiveAction(src, msg);
    switch (tag)
    {
    case Tag::REQUEST:
        if (interestedCS(msg, m_Myts, src, m_Id))   
            App::SendMsg(src, Tag::OK, m_Clock.GetValue());
        else                                        
            m_PendingQ.push(src);
        break;
    case Tag::OK:
        m_NumOkey++;
        cv_Wait.notify_all();   
        break;
    case Tag::BEGIN:
        cv_Connect.notify_all();
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
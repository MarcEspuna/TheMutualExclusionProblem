#include "RAMutex.h"
#include "Log.h"
#include "App.h"
#undef max

static bool interestedCS(int otherTicks, int myTicks, int otherID, int myId);

RAMutex::RAMutex(const Linker& link)
    : Lock(link), m_NumOkey({0}), m_Myts(std::numeric_limits<int>::max()), m_Clock(), m_NumFinished(0)
{
    /* Wait for all the connections */
    {
        std::unique_lock<std::mutex> lck(mtx_Wait);
        cv_Connect.wait(lck, [&](){return m_CurrentComms.size() >=2;});
    }

    LOG_WARN("All connections accepted\n");

    /* State others that we are ready */
    BroadcastMsg(Tag::OK, 0);

    /* Wait for others to be ready */
    {
        std::unique_lock<std::mutex> lck(mtx_Wait);
        cv_Wait.wait(lck, [&](){return m_NumOkey >= 2;});
    }

    m_NumFinished = m_NumOkey;
    LOG_WARN("All processes ready, moving on\n");
}

RAMutex::~RAMutex()
{
    LOG_WARN("Shutting down RA mutex.\n");
    BroadcastMsg(Tag::END, 0);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    LOG_TRACE("Waiting processes to finished.\n");
    cv_Wait.wait(lck, [&](){return m_NumFinished == 0;});
    LOG_TRACE("All processes finished.\n");
}


void RAMutex::requestCS()
{
    LOG_TRACE("Request CS.\n");
    m_Clock.Tick();
    m_Myts = m_Clock.GetValue();
    BroadcastMsg(Tag::REQUEST, m_Myts);
    m_NumOkey = 0;
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
    case Tag::END:
        m_NumFinished--;
        cv_Wait.notify_all();
        break;
    default:
        break;
    }
    cv_Connect.notify_all();
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
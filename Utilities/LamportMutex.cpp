#include "LamportMutex.h"

template<typename T>
static void QueueErase(std::priority_queue<T>& queue);
static bool isGreater(int entry1, int id1, int entry2, int id2);


LamportMutex::LamportMutex(const Linker& link)
   : MsgHandler(link), m_Clock(link.serverPort), m_Id(link.serverPort)
{
    
}   


LamportMutex::~LamportMutex()
{

}


void LamportMutex::requestCS()
{
    m_Clock.Tick();
    int ticks = m_Clock.GetValue(m_Id);
    m_RequestQ.push({ticks, m_Id});
    BroadcastMsg(Tag::REQUEST, ticks);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return this->okeyCS();});
}


void LamportMutex::releaseCS()
{
    auto& [ticks, id] = m_RequestQ.top();
    BroadcastMsg(Tag::RELEASE, m_Clock.GetValue(m_Id));
    m_RequestQ.pop();
}



void LamportMutex::HandleMsg(int message, int src, Tag tag)
{
    m_Clock.RecieveAction(src,message);
    switch (tag)
    {
    case Tag::REQUEST:
        m_RequestQ.push({message, src});
        SendMsg(src, Tag::ACK, m_Clock.GetValue(m_Id));
        break;
    case Tag::RELEASE:
        m_RequestQ.pop();
        break;
    default:
        break;
    }
    cv_Wait.notify_all();
}


void LamportMutex::HandleChildMsg(int message, int src, Tag tag)
{
    HandleMsg(message, src, tag);
}     


bool LamportMutex::okeyCS()
{   
    /* We simply look if we are the best candidate in the priority queue */
    auto& [lowest_Ticks, lowest_Ticks_Id] = m_RequestQ.top();
    if (lowest_Ticks_Id != m_Id)
        return false;
    int myTicks = m_Clock.GetValue(m_Id);
    for (auto& [id, ticks] : m_Clock)
    {
        if (isGreater(m_Id, myTicks, id, ticks))    
            return false;
    }
    return true;
}   

template<typename T>
static void QueueErase(std::priority_queue<T>& queue, T element)
{
    std::priority_queue<T> newQueue;
    while (!queue.empty())
    {
        T& move = queue.top();
        queue.pop();
        if (element != move)
            newQueue.push(element);
    }
    queue = newQueue;
}

static bool isGreater(int entry1, int id1, int entry2, int id2)
{
    return (entry1 > entry2) || (entry1 == entry2 && id1 > id2);
}
#include "LamportMutex.h"

template<typename... Params>
static void QueueErase(std::priority_queue<Params...>& queue, std::pair<int,int> element);
static bool isGreater(int entry1, int id1, int entry2, int id2);


LamportMutex::LamportMutex(const Linker& link)
   : MsgHandler(link), m_Clock(link.serverPort), m_ConnReadyCount(0)
{
    /* Adding the connections to direct clock */
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return ConnectionSize() >=2;});

    LOG_WARN("All connections accepted\n");

    /* State others that we are ready */
    BroadcastMsg(Tag::OK, 0);
    /* Wait for others to be ready */
    cv_Wait.wait(lck, [&](){return m_ConnReadyCount >= 2;});
    LOG_WARN("All processes ready, moving on\n");
}   

LamportMutex::~LamportMutex()
{
    /* Notify all other porcesses that we have finished */
    BroadcastMsg(Tag::END, 0);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    /* Wait for all other processes to finish */
    cv_Wait.wait(lck, [&](){return !m_ConnReadyCount;});
}


void LamportMutex::requestCS()
{
    m_Clock.Tick();
    int ticks = m_Clock.GetValue(m_Id);
    m_RequestQ.push({ticks, m_Id});
    BroadcastMsg(Tag::REQUEST, ticks);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return this->okeyCS();});   // Fix
}


void LamportMutex::releaseCS()
{
    auto& [ticks, id] = m_RequestQ.top();
    BroadcastMsg(Tag::RELEASE, m_Clock.GetValue(m_Id));
    m_RequestQ.pop();
}



void LamportMutex::HandleMsg(int message, int src, Tag tag)
{
    LOG_TRACE("Message, tag: {}, ticks {}\n", (char)tag, message);
    m_Clock.RecieveAction(src,message);
    switch (tag)
    {
    case Tag::REQUEST:
        m_RequestQ.push({message, src});        // Possible problem. Push and erase at the same time
        SendMsg(src, Tag::ACK, m_Clock.GetValue(m_Id));
        break;
    case Tag::RELEASE:
        QueueErase(m_RequestQ, std::make_pair(0,src));
        break;
    case Tag::OK:   // Other process has all it's connections ready
        m_ConnReadyCount++;
        break;
    case Tag::END:  // Other process has finished it's execution
        m_ConnReadyCount--;
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
    /* We look if we are the best candidate in the priority queue */
    auto& [lowest_Ticks, lowest_Ticks_Id] = m_RequestQ.top();
    if (lowest_Ticks_Id != m_Id)
        return false;
    LOG_WARN("First on the queue. ID: {}\n", m_Id);
    int myTicks = m_Clock.GetValue(m_Id);
    for (auto id : m_CurrentComms)
    {
        int ticks = m_Clock.GetValue(id);
        if (isGreater(myTicks, m_Id, ticks, id))    
            return false;
    }
    LOG_WARN("Entering mutex area. ID: {}\n", m_Id);
    return true;
}   

template<typename... Params>
static void QueueErase(std::priority_queue<Params...>& queue, std::pair<int,int> element)
{
    std::priority_queue<Params...> newQueue;
    while (!queue.empty())
    {
        std::pair<int,int> move = queue.top();
        queue.pop();
        if (element.second != move.second)
            newQueue.push(move);
    }
    queue = newQueue;
}

static bool isGreater(int entry1, int id1, int entry2, int id2)
{
    return (entry1 > entry2);
}
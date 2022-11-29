//#define ACTIVE_LOGGING

#include "LamportMutex.h"
#include "App.h"

template<typename... Params>
static void QueueErase(std::priority_queue<Params...>& queue, std::pair<int,int> element);
static bool isGreater(int entry1, int id1, int entry2, int id2);


LamportMutex::LamportMutex(const Linker& link)
   : Lock(link), m_Clock(link.serverPort), m_ConnReadyCount(0)
{
    /* Waitting for all connections */    
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return m_CurrentComms.size() >=2;});

    LOG_WARN("All connections accepted\n");
    App::SendMsg(m_ParentId,Tag::READY);  // Notify parent that we are ready
}   

LamportMutex::~LamportMutex()
{
    /* Notify all other porcesses that we have finished */
    LOG_WARN("Deleting lamport mutex.\n");
    BroadcastMsg(Tag::END, 0);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    /* Wait for all other processes to finish */
    cv_Wait.wait(lck, [&](){return !m_ConnReadyCount;});
    LOG_WARN("All connections closed. Shutting down mutex.\n");
}


void LamportMutex::requestCS()
{
    LOG_WARN("LamportMutex request CS. ID: {}\n", m_Id);
    m_Clock.Tick();
    int ticks = m_Clock.GetValue(m_Id);
    m_RequestQ[m_Id] = ticks;
    BroadcastMsg(Tag::REQUEST, ticks);
    std::unique_lock<std::mutex> lck(mtx_Wait);
    cv_Wait.wait(lck, [&](){return this->okeyCS();});   // Fix
}


void LamportMutex::releaseCS()
{
    LOG_INFO("LamportMutex releasing mutex. ID: {}\n", m_Id);
    m_RequestQ.erase(m_Id);
    BroadcastMsg(Tag::RELEASE, m_Clock.GetValue(m_Id));
}



void LamportMutex::HandleMsg(int message, int src, Tag tag)
{
    LOG_TRACE("Message, tag: {}, ticks {}, from {}\n", (char)tag, message, src);
    m_Clock.RecieveAction(src,message);
    switch (tag)
    {
    case Tag::REQUEST:
        m_RequestQ[src] = message;        
        App::SendMsg(src, Tag::ACK, m_Clock.GetValue(m_Id));
        break;
    case Tag::RELEASE:
        m_RequestQ.erase(src);
        break;
    case Tag::OK:   // Other process has all it's connections ready
        m_ConnReadyCount++;
        break;
    case Tag::END:  // Other process has finished it's execution
        m_ConnReadyCount--;
        break;
    case Tag::BEGIN:
        m_Begin = true;
        cv_Connect.notify_all();
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
    /* We look if we are the best candidate in the priority queue */
    LOG_WARN("Checking CS. ID: {}\n", m_Id);
    if (m_RequestQ.find(m_Id) != m_RequestQ.end())
    {
        int myTicks = m_RequestQ[m_Id];
        for (auto otherId : m_CurrentComms)
        {
            if (m_RequestQ.find(otherId) != m_RequestQ.end())
            {
                if (isGreater(myTicks, m_Id, m_RequestQ[otherId], otherId))
                    return false;
            }
            if (isGreater(myTicks, m_Id, m_Clock.GetValue(otherId), otherId))    
                return false;
        }
    }
    LOG_INFO("Entering mutex area. ID: {}\n", m_Id);
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
    return entry1 > entry2 || (entry1 == entry2 && id1 > id2);
}
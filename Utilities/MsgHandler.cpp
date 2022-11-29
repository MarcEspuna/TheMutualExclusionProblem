#include "MsgHandler.h"
#include "App.h"
//#define ACTIVE_LOGGING
#include "Log.h"

template<typename T>
static bool eraseFromVector(T toErase, std::vector<T>& vec)
{
    auto it = std::find(vec.begin(), vec.end(), toErase);
    if (it != vec.end())
    {
        vec.erase(it);
        return true;
    }   
    return false;
}


MsgHandler::MsgHandler(const Linker& comms)
 :  m_ParentId(comms.parentPort), m_Running(true), 
    m_Id(comms.serverPort), m_ChildFinishes(0), m_Begin(false)
{  
    /* Connect to specified servers */
    for (int port : comms.connections)
    {
        AddClient(port);                                   // Creates a client and connects to specified port server                                     
        StartClientService(port);                          // Starts the thread that handles receptions on this particular client
    }
}

MsgHandler::~MsgHandler()
{
    LOG_TRACE("MsgHandler, closing all clients\n");
    for (int id : m_CurrentComms)
        App::RemoveClient(id);
    App::RemoveClient(m_ParentId);
    for (auto& thread : threads)
        thread.wait();
    LOG_WARN("MsgHandler, End.\n");
}

/// @brief Single thread dedicated to each connected client
/// @param socket Actual client we are looking for
void MsgHandler::ClientService(int id, std::function<void(int, int, Tag)> handleMsg)
{
    bool connected = true;
    while (connected)
    {
        switch (App::IncommingReadFrom(id))
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            std::array<char, 5> data;                           // Reception buffer
            App::ReceiveMsg(id, data);
            {
                std::lock_guard<std::mutex> lck(mtx_CallbackWait);
                handleMsg(*(int*)&data[1], id, (Tag)data[0]);   // Handle message callback
            }
            break;
        case 0:     // Reception closed
            connected = false;
            LOG_WARN("ClientService, Client disconnected, id: {}, my id: {}\n", id, m_Id);
            break;
        default:
            if (WSAGetLastError() != WSAETIMEDOUT)
            {
                LOG_ERROR("ClientService, recv failed with code {}, and id, {}, my id {}\n", WSAGetLastError(), id, m_Id);
                connected = false;
            }
            break;
        } 
    }
    LOG_TRACE("ClientService, removing client.\n");
    App::RemoveClient(id);
    eraseFromVector(id, m_CurrentComms);
}


void MsgHandler::BroadcastMsg(Tag tag, int msg)
{
    LOG_WARN("Broadcasting msg tag, {}\n", (char)tag);
    for (int i = 0; i < m_CurrentComms.size(); i++)
        App::SendMsg(m_CurrentComms[i], tag, msg); 
}

void MsgHandler::BroadcastMsgChilds(Tag tag, int msg)
{
    LOG_WARN("Broadcasting msg tag, {}\n", (char)tag);
    //for (int i = 0; i < m_ChildComms.size(); i++)
    //    SendMsg(m_ChildComms[i], tag, msg); 
}

void MsgHandler::WaitForChilds(int count)
{
    std::unique_lock<std::mutex> lck(mtx_Connect);
    cv_Connect.wait(lck, [&](){return m_ChildFinishes >= count;});
}

void MsgHandler::WaitForNeightbours(int count)
{
    std::unique_lock<std::mutex> lck(mtx_Connect);
    cv_Connect.wait(lck, [&]() {return m_CurrentComms.size() >= count; });
}

void MsgHandler::WaitForParent()
{
    std::unique_lock<std::mutex> lck(mtx_Connect);
    cv_Connect.wait(lck, [&]() { return m_Begin; });
}

void MsgHandler::NotifyEndToParent()
{
    m_Begin = false;
    App::SendMsg(m_ParentId, Tag::READY);
}

void MsgHandler::AddClient(int id)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    m_CurrentComms.push_back(id);
}

void MsgHandler::StartClientService(int port)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, port, BIND_CALLBACK(MsgHandler::HandleMsg)));
}

void MsgHandler::eraseClient(int id)
{
    std::lock_guard<std::mutex> lock(mtx_DataLock);
    eraseFromVector(id, m_CurrentComms);
}
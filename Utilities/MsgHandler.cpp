#include "MsgHandler.h"
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
 :  m_ParentId(comms.parentPort), m_Running(true), m_CurrentLevel(true), m_Server(comms.serverPort), connectivity(nullptr), m_Id(comms.serverPort), m_Sockets(16)

{  
    /* Start incomming connections thread */
    connectivity = new std::thread(&MsgHandler::IncommingConnections, this);
    /* Connect to specified servers */
    for (int port : comms.connections)
    {
        AddClient(port, &m_CurrentComms);                                   // Creates a client and connects to specified port server                                     
        StartClientService(port, BIND_CALLBACK(MsgHandler::HandleMsg));     // Starts the thread that handles receptions on this particular client
    }
    /* Connect to parent if specified*/
    if (m_ParentId){                                                        // Creates a client and connects to specified port server
        AddClient(m_ParentId);
        StartClientService(m_ParentId, BIND_CALLBACK(MsgHandler::HandleMsg));
        LOG_TRACE("Connected to parent, parent id {}, my id {}\n", comms.parentPort, m_Id);
    }
}

MsgHandler::~MsgHandler()
{
    LOG_TRACE("MsgHandler, closing all clients\n");
    closeClients();
    LOG_TRACE("MsgHandler, Server gracefulclose\n");
    m_Server.gracefulClose();
    m_Server.close();
    LOG_TRACE("MsgHandler, Join server connectivity\n");
    connectivity->join();
    delete connectivity;
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
        assertm(m_Sockets.find(id) != m_Sockets.end(), "Null pointer source!");
        switch (m_Sockets[id].IncommingRead())
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            std::array<char, 5> data;                           // Reception buffer
            m_Sockets[id].Receive(data);                              // Get data
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
    m_Sockets[id].gracefulClose();
    eraseClient(id);
}

void MsgHandler::IncommingConnections()
{   
    while (m_Running)
    {
        m_Server.listenConn();
        SOCKET newS = m_Server.acceptClient();
        if (newS)
        {
            int clientId = AddClient(newS, &m_CurrentComms);
            StartClientService(clientId, BIND_CALLBACK(MsgHandler::HandleMsg));                       // Add new client
            LOG_INFO("Notifying\n");

            LOG_TRACE("Server, accepted connection with id, {}, my id {}\n", clientId, m_Id);
        } else {
            LOG_WARN("IncommingConnections, Server closed or accept failed\n");
            break;
        }
    }
    LOG_WARN("IncommingConnections, Incomming connections exited.\n");
}

void MsgHandler::SendMsg(int dest, Tag tag, int msg)
{
    std::array<char, 5> buffer;
    buffer[0] = (char)tag;
    memcpy_s(&buffer[1], sizeof(int), &msg, sizeof(int));
    assertm(m_Sockets.find(dest) != m_Sockets.end(), "Destination socket not found!");
    m_Sockets[dest].Send(buffer);
    LOG_INFO("Msg sent\n");
}

void MsgHandler::SendMsgParent(Tag tag, int msg)
{
    SendMsg(m_ParentId, tag, msg);
}

void MsgHandler::BroadcastMsg(Tag tag, int msg)
{
    LOG_WARN("Broadcasting msg tag, {}\n", (char)tag);
    assertm(m_CurrentComms.size() >= 2, "Comunications not finished!");
    LOG_WARN("Passed assert.\n")
    for (int i = 0; i < m_CurrentComms.size(); i++)
        SendMsg(m_CurrentComms[i], tag, msg); 
}

int MsgHandler::ConnectionSize()
{
    return (int)m_CurrentComms.size(); 
}

int MsgHandler::AddClient(SOCKET sck, std::vector<int>* level)
{
    std::lock_guard<std::mutex> lck(dataLock);
    std::array<int,1> sckId;
    Socket::Receive(sck, sckId);
    assertm(sckId[0] != 0, "Wrong socket id!");
    m_Sockets.try_emplace(sckId[0], sck);
    if (level)  level->push_back(sckId[0]);
    LOG_TRACE("Connection added from {}, my id {}\n", sckId[0], m_Id);
    return sckId[0];
}

int MsgHandler::AddClient(int port, std::vector<int>* level)
{
    std::lock_guard<std::mutex> lck(dataLock);
    m_Sockets.try_emplace(port, port);
    assertm(m_Sockets[port].Connected(), "Client failed to connect!");
    m_Sockets[port].Send(std::array<int,1>{m_Id});
    if (level)  level->push_back(port);
    LOG_TRACE("Connected to {}, my id {}\n", port, m_Id);
    return port;
}

void MsgHandler::StartClientService(int port, std::function<void(int, int, Tag)> callback)
{
    std::lock_guard<std::mutex> lck(dataLock);
    assertm(m_Sockets.find(port) != m_Sockets.end(), "Socket not found!");
    threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, port, callback));
}

void MsgHandler::eraseClient(int id)
{
    std::lock_guard<std::mutex> lock(dataLock);
    if (!eraseFromVector(id, m_CurrentComms))   eraseFromVector(id, m_ChildComms);
    if (m_Sockets.find(id) != m_Sockets.end())  m_Sockets.erase(id);
}

void MsgHandler::closeClients()
{
    std::lock_guard<std::mutex> lock(dataLock);
    for (auto& it : m_Sockets)
        it.second.gracefulClose();
}
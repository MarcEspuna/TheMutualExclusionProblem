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
 : m_Running(true), m_CurrentLevel(true), m_Server(comms.serverPort), connectivity(nullptr), m_Id(comms.serverPort), m_Sockets(16)
{  

    /* Start incomming connections thread */
    connectivity = new std::thread(&MsgHandler::IncommingConnections, this);

    /* Connect to specified servers */
    std::array<int, 1> sockId;
    for (int port : comms.connections)
    {
        m_Sockets.try_emplace(port, port);
        Socket* cl = &m_Sockets[port];
        if (cl->Connected())
        {
            cl->Send(std::array<int,1>{m_Id});                  // Send server ID
            cl->Receive(sockId);                                // Get client ID
            addClient(sockId[0]);                               // Add new client
        }
    }
    /* Connect to parent */
    if (comms.parentPort){
        m_Parent.Connect(comms.parentPort);                       // Connect to parent 
        m_Parent.Send(std::array<int,1>{m_Id});                   // Send owr id
        m_Parent.Receive(sockId);                                 // Receive his id
        LOG_TRACE("Connected to parent, parent id {}, my id {}\n", sockId[0], m_Id);
        /* Create parent callback */
        auto callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        /* Parent callback, ONLY used for centralized thread */
        threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, sockId[0], &m_Parent, callback));
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
    LOG_TRACE("MsgHandler, close parent.\n");
    if (m_Parent.Connected())     m_Parent.gracefulClose();
    for (auto& thread : threads)
        thread.wait();
    LOG_WARN("MsgHandler, End.\n");
}

/// @brief Single thread dedicated to each connected client
/// @param socket Actual client we are looking for
void MsgHandler::ClientService(int id, Socket* src, std::function<void(int, int, Tag)> handleMsg)
{
    bool connected = true;
    while (connected)
    {
        char check;
        switch (recv(src->getDescriptor(), &check, 1, MSG_PEEK))
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            std::array<char, 5> data;                           // Reception buffer
            src->Receive(data);                              // Get data
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
    src->gracefulClose();
    eraseClient(id);
}

void MsgHandler::IncommingConnections()
{   
    std::array<int, 1> clientId;
    while (m_Running)
    {
        m_Server.listenConn();
        SOCKET newS = m_Server.acceptClient();
        if (newS)
        {
            Socket::Receive(newS, clientId);        // Get client ID
            Socket::Send(newS, std::array<int,1>{m_Id});
            m_Sockets.try_emplace(clientId[0], newS);

            LOG_TRACE("Server, accepted connection with id, {}, my id {}\n", clientId[0], m_Id);
            addClient(clientId[0]);                         // Add new client
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
    Socket* skt = &m_Sockets[dest];
    skt->Send(buffer);
}

void MsgHandler::SendMsgParent(Tag tag, int msg)
{
    std::array<char, 5> buffer;
    buffer[0] = (char)tag;
    memcpy_s(&buffer[1], sizeof(int), &msg, sizeof(int));
    m_Parent.Send(buffer);
}

void MsgHandler::BroadcastMsg(Tag tag, int msg)
{
    for(int id : m_CurrentComms)  SendMsg(id, tag, msg);
}

void MsgHandler::addClient(int id)
{
    std::lock_guard<std::mutex> lock(dataLock);
    std::function<void(int, int, Tag)> callback;
    if (m_CurrentLevel)
    {
        m_CurrentComms.push_back(id);         // Add connection 
        callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    else 
    {
        m_ChildComms.push_back(id);
        callback = std::bind(&MsgHandler::HandleChildMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    /* Start client thread */
    threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, id, &m_Sockets[id], callback));
}

void MsgHandler::eraseClient(int id)
{
    std::lock_guard<std::mutex> lock(dataLock);
    if (!eraseFromVector(id, m_CurrentComms))
        eraseFromVector(id, m_ChildComms);
    m_Sockets.erase(id);
}

void MsgHandler::closeClients()
{
    std::lock_guard<std::mutex> lock(dataLock);
    for (const auto& id : m_CurrentComms)
        m_Sockets[id].gracefulClose();
}
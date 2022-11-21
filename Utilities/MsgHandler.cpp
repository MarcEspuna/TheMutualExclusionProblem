#include "MsgHandler.h"
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
 : m_Running(true), m_CurrentLevel(true), connectivity(nullptr)
{ 
    /* Bind server port */
    server.Bind(comms.serverPort);  
    sockets.insert({server.GetPort(), &server});

    /* Start incomming connections thread */
    connectivity = new std::thread(&MsgHandler::IncommingConnections, this);

    /* Make specified connections */
    std::array<int, 1> sockId;
    for (int port : comms.connections)
    {
        Client* cl = new Client();
        cl->Connect(port);
        if (cl->Connected())
        {
            cl->Send(std::array<int,1>{server.GetPort()});      // Send server ID
            cl->Receive(sockId);                                // Get client ID
            addClient(sockId[0], cl);                           // Add new client
        }
    }
    if (comms.parentPort){
        parent.Connect(comms.parentPort);                       // Connect to parent 
        parent.Send(std::array<int,1>{server.GetPort()});       // Send owr id
        parent.Receive(sockId);                                 // Receive his id
        sockets.insert({sockId[0], &parent});                   // Add parent socket
        LOG_TRACE("Connected to parent, parent id {}, my id {}\n", sockId[0], server.GetPort());
        /* Create parent callback */
        auto callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        /* Parent callback, ONLY used for centralized thread */
        threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, sockId[0], callback));
    }
}

MsgHandler::~MsgHandler()
{
    LOG_TRACE("MsgHandler, closing all clients\n");
    closeClients();
    for (const auto& ids : currentComms)   
        delete sockets[ids];
    for (const auto& ids : childComms)
        delete sockets[ids];

    LOG_TRACE("MsgHandler, Server gracefulclose\n");
    server.gracefulClose();
    server.close();
    LOG_TRACE("MsgHandler, Join server connectivity\n");
    connectivity->join();
    delete connectivity;
    LOG_TRACE("MsgHandler, close parent.\n");
    if (parent.Connected())     parent.gracefulClose();
    LOG_WARN("MsgHandler, End.\n");
}

/// @brief Single thread dedicated to each connected client
/// @param socket Actual client we are looking for
void MsgHandler::ClientService(int id, std::function<void(int, int, Tag)> handleMsg)
{
    bool connected = true;
    Socket* socket = sockets[id];
    while (connected)
    {
        char check;
        switch (recv(socket->getDescriptor(), &check, 1, MSG_PEEK))
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            LOG_TRACE("ClientService, Incomming read.\n");
            std::array<char, 5> data;                           // Reception buffer
            socket->Receive(data);                              // Get data
            handleMsg(*(int*)&data[1], id, (Tag)data[0]);   // Handle message callback
            break;
        case 0:     // Reception closed
            socket->gracefulClose();
            eraseClient(id);
            connected = 0;
            LOG_WARN("ClientService, Client disconnected.\n");
            break;
        default:
            if (WSAGetLastError() != WSAETIMEDOUT)
            {
                LOG_ERROR("ClientService, recv failed with code {}, and socket, {}\n", WSAGetLastError(), socket->getDescriptor());
                socket->gracefulClose();
                eraseClient(id);
                connected = 0;
            }
            break;
        } 
    }
}

void MsgHandler::IncommingConnections()
{   
    std::array<int, 1> clientId;
    while (m_Running)
    {
        server.listenConn();
        SOCKET newS = server.acceptClient();
        if (newS)
        {
            Client* cl = new Client(newS, {});
            cl->Receive(clientId);                              // Get client ID
            cl->Send(std::array<int,1>{server.GetPort()});      // Send server ID
            LOG_TRACE("Server, accepted connection with id, {}, my id {}\n", clientId[0], server.GetPort());
            addClient(clientId[0], cl);                         // Add new client
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
    Socket* skt = sockets[dest];
    skt->Send(buffer);
}

void MsgHandler::SendMsgParent(Tag tag, int msg)
{
    std::array<char, 5> buffer;
    buffer[0] = (char)tag;
    memcpy_s(&buffer[1], sizeof(int), &msg, sizeof(int));
    parent.Send(buffer);
}

void MsgHandler::BroadcastMsg(Tag tag, int msg)
{
    for(int id : currentComms)  SendMsg(id, tag, msg);
}

void MsgHandler::addClient(int id, Socket* client)
{
    std::lock_guard<std::mutex> lock(dataLock);
    std::function<void(int, int, Tag)> callback;
    if (m_CurrentLevel)
    {
        currentComms.push_back(id);         // Add connection 
        callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    else 
    {
        childComms.push_back(id);
        callback = std::bind(&MsgHandler::HandleChildMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    /* Add socket to hash map */
    sockets.insert({id, client});
    /* Start client thread */
    threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, id, callback));
}

void MsgHandler::eraseClient(int id)
{
    std::lock_guard<std::mutex> lock(dataLock);
    if (!eraseFromVector(id, currentComms))
        eraseFromVector(id, childComms);
    sockets.erase(id);
}

void MsgHandler::closeClients()
{
    std::lock_guard<std::mutex> lock(dataLock);
    for (const auto& id : currentComms)
        sockets[id]->gracefulClose();
}
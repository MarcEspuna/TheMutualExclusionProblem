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

    /* Start incomming connections thread */
    connectivity = new std::thread(&MsgHandler::IncommingConnections, this);

    /* Make specified connections */
    for (int port : comms.connections)
    {
        Client* cl = new Client();
        cl->Connect(port);
        if (cl->Connected())
            addClient(cl);
    }
    if (comms.parentPort){
        parent.Connect(comms.parentPort);
        auto callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, &parent, callback));
    }
}

MsgHandler::~MsgHandler()
{
    LOG_TRACE("MsgHandler, Closing clients");
    closeClients();
    for (const auto& socket : currentComms)   
        delete socket;

    LOG_TRACE("MsgHandler, Server gracefulclose");
    server.gracefulClose();
    server.close();
    LOG_TRACE("MsgHandler, Join server connectivity");
    connectivity->join();
    delete connectivity;
    LOG_TRACE("MsgHandler, close parent.");
    if (parent.Connected())     parent.gracefulClose();
    LOG_WARN("MsgHandler, End.");
    
}

/// @brief Single thread dedicated to each connected client
/// @param socket Actual client we are looking for
void MsgHandler::ClientService(Socket* socket, std::function<void(int, Socket*, Tag)> handleMsg)
{
    bool connected = true;
    while (connected)
    {
        char check;
        switch (recv(socket->getDescriptor(), &check, 1, MSG_PEEK))
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            LOG_TRACE("ClientService, Incomming read.");
            std::array<char, 5> data;                           // Reception buffer
            socket->Receive(data);                              // Get data
            handleMsg(*(int*)&data[1], socket, (Tag)data[0]);   // Handle message callback
            break;
        case 0:     // Reception closed
            socket->gracefulClose();
            eraseClient(socket);
            connected = 0;
            LOG_WARN("ClientService, Client disconnected.");
            break;
        default:
            if (WSAGetLastError() != WSAETIMEDOUT)
            {
                LOG_ERROR("[RECEPTION HANDLING]: Recv failed with code {}, and socket, {}", WSAGetLastError(), socket->getDescriptor());
                socket->gracefulClose();
                eraseClient(socket);
                connected = 0;
            }
            break;
        } 
    }
}

void MsgHandler::IncommingConnections()
{   
    while (m_Running)
    {
        server.listenConn();
        SOCKET newS = server.acceptClient();
        if (newS)
        {
            Client* cl = new Client(newS, {});
            addClient(cl);
        } else {
            LOG_WARN("IncommingConnections, Server closed or accept failed");
            break;
        }
    }
    LOG_WARN("IncommingConnections, Incomming connections exited.");
}

void MsgHandler::SendMsg(Socket* dest, Tag tag, int msg)
{
    std::array<char, 5> buffer;
    buffer[0] = (char)tag;
    memcpy_s(&buffer[1], sizeof(int), &msg, sizeof(int));
    dest->Send(buffer);
}

void MsgHandler::SendMsgParent(Tag tag, int msg)
{
    SendMsg(&parent, tag, msg);
}

void MsgHandler::addClient(Socket* client)
{
    std::lock_guard<std::mutex> lock(dataLock);
    std::function<void(int, Socket*, Tag)> callback;
    if (m_CurrentLevel)
    {
        currentComms.push_back(client);         // Add connection 
        callback = std::bind(&MsgHandler::HandleMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    else 
    {
        childComms.push_back(client);
        callback = std::bind(&MsgHandler::HandleChildMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    /* Start client thread */
    threads.push_back(std::async(std::launch::async, &MsgHandler::ClientService, this, client, callback));
}

void MsgHandler::eraseClient(Socket* client)
{
    std::lock_guard<std::mutex> lock(dataLock);
    if (!eraseFromVector(client, currentComms))
        eraseFromVector(client, childComms);
}

void MsgHandler::closeClients()
{
    std::lock_guard<std::mutex> lock(dataLock);
    for (const auto& client : currentComms)
        client->gracefulClose();
}
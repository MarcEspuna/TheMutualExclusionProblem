#pragma once
#include "Commons.h"
#include "Server.h"
#include "Client.h"
#include "Socket.h"

enum class Tag {
    REQUEST = 'R', RELEASE = 'L', OK = 'K', ACK = 'A'
};

struct Linker
{
    int serverPort;                     // Own server port                    
    int parentPort;                     // Parent port
    std::vector<int> connections;       // Connections of the client
};

class MsgHandler {
public:
    MsgHandler(const Linker& comms);
    ~MsgHandler();

    void SendMsgParent(Tag tag, int msg = 0);
    void SendMsg(Socket* dest, Tag tag, int msg = 0);

    inline void SetIncommingSocketsToCurrentLevel() { m_CurrentLevel = true; }
    inline void SetIncommingSocketsToChildLevel() { m_CurrentLevel = false; }    

    /* Pure virtual function */
    virtual void HandleMsg(int message, Socket* src, Tag tag) = 0;          /* Used for current level processes */
    virtual void HandleChildMsg(int message, Socket* src, Tag tag) = 0;     /* Used for child porcesses */

private:
    Server server;
    Client parent;                          // Upper level
    std::vector<Socket*> currentComms;      // Current level
    std::vector<Socket*> childComms;        // Child level

    /* Incomming connections thread */
    std::thread* connectivity;
    std::vector<std::future<void>> threads;
    
    std::mutex dataLock;                    // For data management
    std::mutex sendLock;   

    bool m_CurrentLevel;
    bool m_Running;
private:
    void IncommingConnections();
    void ClientService(Socket* socket, std::function<void(int, Socket*, Tag)> callback);

    /* Data management */
    void addClient(Socket* client);
    void eraseClient(Socket* client);
    void closeClients();
};


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
    void SendMsg(int dest, Tag tag, int msg = 0);
    void BroadcastMsg(Tag tag, int msg);

    inline void SetIncommingSocketsToCurrentLevel() { m_CurrentLevel = true; }
    inline void SetIncommingSocketsToChildLevel() { m_CurrentLevel = false; }    

    /* Pure virtual function */
    virtual void HandleMsg(int message, int src, Tag tag) = 0;          /* Used for current level processes */
    virtual void HandleChildMsg(int message, int src, Tag tag) = 0;     /* Used for child porcesses */

private:
    Server server;                              // Incomming connections                            
    Client parent;                              // Upper level connection

    std::unordered_map<int, Socket*> sockets;
    std::vector<int> currentComms;              // Current level socket ids
    std::vector<int> childComms;                // Child level socket ids

    /* Incomming connections thread */
    std::thread* connectivity;
    std::vector<std::future<void>> threads;
    
    std::mutex dataLock;                        // For data management
    std::mutex sendLock;   

    bool m_CurrentLevel;
    bool m_Running;
private:
    void IncommingConnections();
    void ClientService(int id, std::function<void(int, int, Tag)> callback);

    /* Data management */
    void addClient(int id, Socket* client);
    void eraseClient(int id);
    void closeClients();
};


#pragma once
#include "Commons.h"
#include "Server.h"
#include "Client.h"
#include "Socket.h"

enum class Tag {
    REQUEST = 'R', RELEASE = 'L', OK = 'K', ACK = 'A', END='E'
};

struct Linker
{
    int serverPort;                     // Own server port                    
    int parentPort;                     // Parent port
    int totalConnections;               // Total number of connections
    std::vector<int> connections;       // Connections of the client
};

class MsgHandler {
public:
    MsgHandler(const Linker& comms);
    virtual ~MsgHandler();

    void SendMsgParent(Tag tag, int msg = 0);
    void SendMsg(int dest, Tag tag, int msg = 0);
    void BroadcastMsg(Tag tag, int msg);

    inline void SetIncommingSocketsToCurrentLevel() { m_CurrentLevel = true; }
    inline void SetIncommingSocketsToChildLevel() { m_CurrentLevel = false; }    

    /* Pure virtual function */
    virtual void HandleMsg(int message, int src, Tag tag) = 0;          /* Used for current level processes */
    virtual void HandleChildMsg(int message, int src, Tag tag) = 0;     /* Used for child porcesses */

private:
    int m_ParentId;                                 // Upper level connection
    Server m_Server;                              // Incomming connections                            

    std::unordered_map<int, Client> m_Sockets;
    std::vector<int> m_ChildComms;                // Child level socket ids

    /* Incomming connections thread */
    std::thread* connectivity;
    std::vector<std::future<void>> threads;
    
    std::mutex dataLock;                        // For data management 

    bool m_CurrentLevel;
    bool m_Running;

protected:
    int m_Id;  
    std::vector<int> m_CurrentComms;              // Current level socket ids

protected:
    int ConnectionSize();
    void IncommingConnections();
private:
    /* For MshHandler syncronization */
    std::mutex mtx_CallbackWait;

    void StartClientService(int port, std::function<void(int, int, Tag)> callback);
    void ClientService(int id, std::function<void(int, int, Tag)> callback);
    /* Data management */
    int AddClient(int port, std::vector<int>* level = nullptr);
    int AddClient(SOCKET sck, std::vector<int>* level= nullptr);
    
    void eraseClient(int id);
    void closeClients();
};


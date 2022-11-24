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

    std::vector<int>::iterator begin() { return m_CurrentComms.begin(); }
    std::vector<int>::iterator end() { return m_CurrentComms.end(); }

private:
    Server m_Server;                              // Incomming connections                            
    Client m_Parent;                              // Upper level connection

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
    inline int ConnectionSize() {return (int)m_CurrentComms.size(); }
    void IncommingConnections();
private:
    /* For MshHandler syncronization */
    std::mutex mtx_CallbackWait;

    void ClientService(int id, Socket* src, std::function<void(int, int, Tag)> callback);
    /* Data management */
    void addClient(int id);
    void eraseClient(int id);
    void closeClients();
};


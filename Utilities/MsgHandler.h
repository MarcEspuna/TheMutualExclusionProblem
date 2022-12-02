#pragma once
#include "Commons.h"
#include "Server.h"
#include "Client.h"
#include "Socket.h"

enum class Tag {
    REQUEST = 'R', RELEASE = 'L', OK = 'K', ACK = 'A', END='E', BEGIN='B', TERMINATE='T', READY='D'
};

struct Linker
{
    int serverPort;                     // Own server port                    
    int parentPort;                     // Parent port
    int totalConnections;               // Total number of connections
    std::vector<int> connections;       // Connections of the client

    std::vector<std::string> GetStrConnections() const {
        std::vector<std::string> ports;
        ports.reserve(connections.size());
        for (int i : connections)
            ports.push_back(std::to_string(i));
        return ports;
    } 
};


class MsgHandler {
public:
    MsgHandler(const Linker& comms);
    virtual ~MsgHandler();

    void BroadcastMsg(Tag tag, int msg);

    /* Pure virtual function */
    virtual void HandleMsg(int message, int src, Tag tag) = 0;          /* Used for current level processes */
    virtual void HandleChildMsg(int message, int src, Tag tag) = 0;     /* Used for child porcesses */

    void AddClient(int id);
    void StartClientService(int port);
protected:
    int m_Id;  
    int m_ParentId;                                 // Upper level connection                        
    std::vector<int> m_CurrentComms;              // Current level socket ids

    int m_ChildFinishes;                          // Number of child processes that have finished
    bool m_Begin;

    std::mutex mtx_Connect;
    std::condition_variable cv_Connect;
private:
    /* Incomming connections thread */
    std::vector<std::thread*> threads;
    
    std::mutex mtx_DataLock;                        // For data management 

    bool m_Running;
    
private:

    /* For MshHandler syncronization */
    std::mutex mtx_CallbackWait;

    void ClientService(int id, std::function<void(int, int, Tag)> callback);
    void eraseClient(int id);
};


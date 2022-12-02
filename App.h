#pragma once
#include "Commons.h"
#include "Utilities/Lock.h"
#include "Utilities/Log.h"

class App : public Server {
public:
    template<typename T>
    static void Create(const std::string& name, const Linker& link, MtxType mtxType);
    
    static App* Get() { return s_App; };
    static void Run() { s_App->run(); }
    static void Destroy();
    static int IncommingReadFrom(int id);
    
    template<typename T, int S>
    static int ReceiveMsg(int src, std::array<T,S>& data);
    static void SendMsg(int dest, Tag tag, int msg = 0);
    
    static void RemoveClient(int id);

    static size_t GetConnSize() { return s_App->m_Sockets.size(); }
protected:
    App(const std::string& name, const Linker& link, MtxType mtxType);
    virtual ~App();

    virtual void IncommingConnection(SOCKET client) = 0;
    virtual void run() = 0;

    void BroadcastMsgToChilds(Tag tag, int msg = 0);

    int AddClient(SOCKET client);  
    int AddClient(int port);
protected:
    int m_Id;
    int m_ParentId;
    std::string m_Name;
    Lock* m_Mutex;

    bool m_ClientsToLock;                           // Flag that indicates to save the incomming connections to the mutex lock
    std::unordered_map<int, Client> m_Sockets;
    
    /* Singleton */
    inline static App* s_App = nullptr;

    std::mutex mtx_DataLock;
};


template<typename T>
void App::Create(const std::string& name, const Linker& link, MtxType mtxType)
{
    if (!s_App) {
        Log::CreateLogger(name);
        Socket::Init();
        s_App = new T(name, link, mtxType); 
    }
}

template<typename T, int S>
int App::ReceiveMsg(int src, std::array<T,S>& data) 
{ 
    assertm(s_App->m_Sockets.find(src) != s_App->m_Sockets.end(), "Socket not found!"); 
    return s_App->m_Sockets.at(src).Receive(data);    
}
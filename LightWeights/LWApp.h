#pragma once
#include "Commons.h"
#include "Server.h"
#include "Lock.h"
#include "App.h"

class LWApp : public Server {
public:
    static void Create(const std::string& name, const Linker& link, MtxType mtxType);
    static LWApp* Get() { return lwApp; };
    static void Run() { lwApp->run(); }
    static void Destroy();
    static void SendMsgParent(Tag tag, int msg = 0);

private:
    LWApp(const std::string& name, const Linker& link, MtxType mtxType);
    ~LWApp();

    int m_Id;
    std::string m_Name;
    Lock* m_Mutex;

    bool m_ClientsToLock;                           // Flag that indicates to save the incomming connections to the mutex lock
    std::unordered_map<int, Client> m_Childs;
    Client m_Parent;
    
    inline static LWApp* lwApp = nullptr;
private:
    void IncommingConnection(SOCKET client) override;
    void run();
};
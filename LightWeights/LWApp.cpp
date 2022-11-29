#include "LWApp.h"
#include "CentMutex.h"
#include "LamportMutex.h"
#include "RAMutex.h"
#include "Log.h"
#include "io.h"

/**
 * @brief The app constructor will bind a server and attempt to connect to it's specified parent server
 * 
 * @param name 
 * @param link 
 * @param mtxType 
 */
LWApp::LWApp(const std::string& name, const Linker& link,  MtxType mtxType)
    : m_Name(name), m_Id(link.serverPort), m_Mutex(nullptr), Server(link.serverPort), m_ClientsToLock(true), m_Parent(link.parentPort)
{
    assertm(m_Parent.Connected(), "Could not connect to parent!");
    m_Parent.Send(std::array<int,1>{m_Id});
    /* Init windows sockets */
    switch (mtxType)
    {
        case MtxType::LAMPORT:
            m_Mutex = Lock::Create<LamportMutex>(link);
            break;
        case MtxType::RICART_AGRAWALA:
            m_Mutex = Lock::Create<RAMutex>(link);
            break;
        default:
            LOG_ERROR("Unsuported mutex type.\n");
            break;
    }
}

LWApp::~LWApp()
{
    delete m_Mutex;
    Log::EndLogging();
}

void LWApp::Create(const std::string& name, const Linker& link, MtxType mtxType)
{
    if (!lwApp) {
        Log::CreateLogger(name);
        Socket::Init();
        lwApp = new LWApp(name, link, mtxType); 
    }
}

void LWApp::Destroy()
{
    delete lwApp; 
    Socket::Finit(); 
    Log::EndLogging();
}

void LWApp::run()
{
    LOG_INFO("Main app run\n");
    /* Not as leader */

    // Wait for begin of parent

    if (m_Mutex)
    {
        m_Name.append("\n");
        for (int i = 0; i < 5; i++)
        {
            m_Mutex->requestCS();
            Sleep(1000);
            _write(1, m_Name.c_str(), (int)m_Name.size());
            m_Mutex->releaseCS();
        }
    } 
    // Loop until terminate is received from parent

    LOG_INFO("Exit app run.\n");
}

void LWApp::IncommingConnection(SOCKET client)
{
    if (m_ClientsToLock)    
    {
        //int clId = AddClient(client);
        //m_Mutex->AddClient(clId);
        //m_Mutex->StartClientService(clId);
    }else 
    {
        std::array<int,1> sckId;
        Socket::Receive(client, sckId);
        assertm(sckId[0] != 0, "Wrong socket id!");
        m_Childs.try_emplace(sckId[0], client);
        LOG_TRACE("Connection added from {}, my id {}\n", sckId[0], m_Id);
    }
}
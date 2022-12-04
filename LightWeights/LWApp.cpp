#include "LWApp.h"
#include "CentMutex.h"
#include "LamportMutex.h"
#include "RAMutex.h"
#include "Log.h"
#include "io.h"

static const char* consoleMsg = "Soc el proces lightweight: ";

/**
 * @brief The app constructor will bind a server and attempt to connect to it's specified parent server
 * 
 * @param name 
 * @param link 
 * @param mtxType 
 */
LWApp::LWApp(const std::string& name, const Linker& link,  MtxType mtxType)
    : App(name, link, mtxType), test(link)
{    
    /* Start specified mutex */
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
    /* Start handling incomming connections */
    StartConnectionHandling();
    
    /* Connect to all specifed connections */
    for (int port: link.connections)
        AddClient(port);
}

LWApp::~LWApp()
{
    delete m_Mutex;
}

static std::mutex mtx_ConnWait;
static std::condition_variable cv_ConnWait;

void LWApp::run()
{
    for (int port : test.connections)
    {
        m_Mutex->AddClient(port);                                   // Creates a client and connects to specified port server                                     
        m_Mutex->StartClientService(port);                          // Starts the thread that handles receptions on this particular clien
    }

    /* Notify parent that we are ready */
    LOG_TRACE("Waitting cons\n");
    {
        std::unique_lock<std::mutex> lck(mtx_ConnWait);
        cv_ConnWait.wait(lck, [&](){return App::GetConnSize() >= test.totalConnections;});
    }
    LOG_TRACE("Notifying parent that we are ready\n");
    App::SendMsg(m_ParentId, Tag::READY, 0);
    LOG_INFO("Waiting for begin\n");
    std::array<char, 5> data;
    App::ReceiveMsg(m_ParentId, data);
    LOG_TRACE("Something received!\n");
    // Wait for begin of parent
    std::string prompt(consoleMsg);
    prompt.append(m_Name);
    prompt.append("\n");

    while (((Tag)data[0]) == Tag::BEGIN)
    {
        LOG_TRACE("Begin recived.\n");
        for (int i = 0; i < 3; i++)
        {
            m_Mutex->requestCS();
            Sleep(2);
            _write(1, prompt.c_str(), (int)prompt.size());
            m_Mutex->releaseCS();
        }
        App::SendMsg(m_ParentId, Tag::READY, 0);
        App::ReceiveMsg(m_ParentId, data);
    }
    
    // Loop until terminate is received from parent
    LOG_INFO("Exit app run.\n");
}

void LWApp::IncommingConnection(SOCKET client)
{
    int clientId = AddClient(client);
    m_Mutex->AddClient(clientId);
    m_Mutex->StartClientService(clientId);
    LOG_TRACE("Connection size before notify: {}\n", App::GetConnSize());
    cv_ConnWait.notify_all();
}
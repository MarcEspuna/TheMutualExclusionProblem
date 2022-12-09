#include "HWApp.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
#include "Log.h"
#include "io.h"

static const char* consoleMsg = "Soc el proces heavyweight: ";
static void printProcess(std::string pName);

HWApp::HWApp(const std::string& name, const Linker& link, MtxType mtxType)
    : App(name, link, mtxType), m_ChildFinishes(0)
{ 

    m_Mutex = Lock::Create<CentMutex>(link, false);         // Not as leader

    LOG_INFO("Creating lightweight processes\n");
    m_Processes.reserve(5);

    std::vector<std::string> arguments = link.GetProcessArgs(name, "LW.exe", mtxType);
    for (const std::string& arg : arguments)
    {
        std::cout << arg << std::endl;
        m_Processes.emplace_back();
        m_Processes.back().launch(arg.c_str());
    }
    StartConnectionHandling();
}

HWApp::~HWApp()
{
    delete m_Mutex;
}

void HWApp::run()
{
    m_Mutex->StartClientService(m_ParentId);
    
    std::string prompt(consoleMsg);
    prompt.append(m_Name);

    /* *** Child Connection startup *** */
    LOG_TRACE("Waiting for childs to be ready\n");
    WaitForChilds();   
    for (int i = 0; i < 20; i++)
    {
        m_Mutex->requestCS();                           // Request Token and wait for Tocken
        printProcess(prompt);
        NotifyChildsToStart();                          // Send start to all childs
        LOG_TRACE("Waiting for childs to be ready\n");
        WaitForChilds();                               // Wait for all childs to finish
        LOG_TRACE("Releasing.\n");
        m_Mutex->releaseCS();                           // Return token to leader
    }

    BroadcastMsgToChilds(Tag::TERMINATE);               // Tell childs to finish
    LOG_INFO("Waiting child processes to finish\n");
    for (Process& process : m_Processes)
        process.wait();
}

/**
 * @brief Function called when a new client is connecting to our server. For the heavy weight app all incomming connections will be considered
 * connections from child processes.
 * @param client the file descriptor from the incomming client
 */
void HWApp::IncommingConnection(SOCKET client)          // This connections are comming from child processes
{
    int clientID = App::AddClient(client);
    std::thread notifyClientReady(&HWApp::ChildReadyNotify, this, clientID);
    notifyClientReady.detach();
}

void HWApp::WaitForChilds()
{
    std::unique_lock<std::mutex> lck(mtx_Childs);
    LOG_TRACE("Start wait.\n");
    cv_Childs.wait(lck, [&](){return m_ChildFinishes >= m_Processes.size();});
    LOG_TRACE("Stoped waiting!\n");
}

void HWApp::ChildReadyNotify(int id)
{
    assertm(m_Sockets.find(id) != m_Sockets.end(), "Null pointer source!");
    switch (m_Sockets[id].IncommingRead())
    {
    case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
        std::array<char, 5> data;                                 // Reception buffer
        m_Sockets[id].Receive(data);                              // Get data
        if (((Tag)data[0]) == Tag::READY){
            LOG_TRACE("Child {} is ready.\n", id);
            {
                std::lock_guard<std::mutex> lck(mtx_Data);
                m_ChildFinishes++;
            }
            cv_Childs.notify_all();
        }
        break;
    case 0:
        LOG_WARN("ChildNotify, socket disconnected.\n");
        break;
    default:
        if (WSAGetLastError() != WSAETIMEDOUT)
        {
            LOG_ERROR("ChildNotify, recv failed with code {}, and id, {}, my id {}\n", WSAGetLastError(), id, GetPort());
        }
        break;
    } 
}

void HWApp::NotifyChildsToStart()
{
    LOG_TRACE("Notifying childs to start\n");
    m_ChildFinishes = 0;
    LOG_TRACE("Broadcasting begin to childs");
    BroadcastMsgToChilds(Tag::BEGIN);
    for(const auto& [id, client] : m_Sockets)
    {
        if (id != m_ParentId)
        {
            std::thread notifyClientReady(&HWApp::ChildReadyNotify, this, id);
            notifyClientReady.detach();
        }
    }
}

void HWApp::BroadcastMsgToChilds(Tag tag, int msg)
{
    for(const auto& [id, client] : m_Sockets)
    {
        if (id != m_ParentId)   App::SendMsg(id, tag, msg);
    }
}

static void printProcess(std::string pName)
{
    pName.append("\n");
    std::cout << pName;
}
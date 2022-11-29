#include "HWApp.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
#include "Log.h"
#include "io.h"

static void printProcess(std::string pName);

HWApp::HWApp(const std::string& name, const Linker& link, MtxType mtxType)
    : App(name, link, mtxType), m_ChildFinishes(0)
{
    /* Init windows sockets */
    Linker centLink = link;
    centLink.connections.clear();   

    m_Mutex = Lock::Create<CentMutex>(centLink, false);         // Not as leader
    m_Mutex->StartClientService(link.parentPort);

    m_Processes.reserve(5);
    for (int i = 0; i < 3; i++)
        m_Processes.emplace_back("LW.exe");

    std::string name1 = m_Name;
    std::string name2 = m_Name;
    std::string name3 = m_Name;

    name1.append("-LW_1");
    name2.append("-LW_2");
    name3.append("-LW_3");

    std::vector<std::string> childPorts = link.GetStrConnections();
    std::string strMtxType = (mtxType == MtxType::LAMPORT) ? std::string(MTX_LAMPORT) : std::string(MTX_RA); 

    //              <name>    <mtx type>  <child server port>       <parent port(me)>                  <connection count>    <ports for the child to connect to>
    //m_Processes[0].launch({name1.c_str(), strMtxType.c_str(), childPorts[0].c_str(), std::to_string(GetPort()).c_str(),    "2"});
    //m_Processes[1].launch({name2.c_str(), strMtxType.c_str(), childPorts[1].c_str(), std::to_string(GetPort()).c_str(),    "2",             childPorts[0].c_str()});
    //m_Processes[2].launch({name3.c_str(), strMtxType.c_str(), childPorts[2].c_str(), std::to_string(GetPort()).c_str(),    "2",             childPorts[0].c_str(), childPorts[1].c_str()});
}

HWApp::~HWApp()
{
    delete m_Mutex;
    Socket::Finit();    // Finalize windows sockets
    Log::EndLogging();
}

void HWApp::run()
{
    std::string name(m_Name);
    name.append("\n");
    LOG_INFO("Main app run\n");
    /* *** Child Connection startup *** */
    //WaitForChilds(3);                                   // Wait for all childs to be ready
    
    for (int i = 0; i < 30; i++)
    {
        m_Mutex->requestCS();                           // Request Token and wait for Tocken
        std::cout << name;
        Sleep(1);
        //printProcess(m_Name);                           // Print our process name
        //NotifyChildsToStart();                          // Send start to all childs
        //WaitForChilds(3);                               // Wait for all childs to finish
        m_Mutex->releaseCS();                           // Return token to leader
    }

    //BroadcastMsgToChilds(Tag::TERMINATE);               // Tell childs to finish
    LOG_INFO("Waiting child processes to finish\n");
    //for (Process& process : m_Processes)
        //process.wait();
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

void HWApp::WaitForChilds(int count)
{
    std::unique_lock<std::mutex> lck(mtx_Childs);
    cv_Childs.wait(lck, [&](){return m_ChildFinishes >= count;});
}

void HWApp::ChildReadyNotify(int id)
{
    assertm(m_Sockets.find(id) != m_Sockets.end(), "Null pointer source!");
    switch (m_Sockets[id].IncommingRead())
    {
    case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
        std::array<char, 5> data;                           // Reception buffer
        m_Sockets[id].Receive(data);                              // Get data
        if ((Tag)data[0] == Tag::READY)
            m_ChildFinishes++;
        break;
    default:
        if (WSAGetLastError() != WSAETIMEDOUT)
        {
            LOG_ERROR("ClientService, recv failed with code {}, and id, {}, my id {}\n", WSAGetLastError(), id, GetPort());
        }
        break;
    } 
}

void HWApp::NotifyChildsToStart()
{
    m_ChildFinishes = 0;
    for(const auto& it : m_Sockets)
    {
        std::thread notifyClientReady(&HWApp::ChildReadyNotify, this, it.first);
        notifyClientReady.detach();
    }
    BroadcastMsgToChilds(Tag::BEGIN);
}

void HWApp::BroadcastMsgToChilds(Tag tag, int msg)
{
    //for(const auto& it : m_Sockets)
    //App::SendMsg(it.second, tag, msg);
}

static void printProcess(std::string pName)
{
    pName.append("\n");
    std::cout << pName;
}
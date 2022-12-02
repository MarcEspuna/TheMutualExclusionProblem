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
    LOG_INFO("Starting mutex client service on parent.\n");

    LOG_INFO("Creating lightweight processes\n");
    m_Processes.reserve(5);
    for (int i = 0; i < 3; i++)
        m_Processes.emplace_back("C:/Users/marce/Documents/UNIVERSITAT/ProjArqDist/Sessio2/ProcessTest/build/Debug/LW.exe");

    std::string name1 = m_Name;
    std::string name2 = m_Name;
    std::string name3 = m_Name;

    name1.append("-LW_1");
    name2.append("-LW_2");
    name3.append("-LW_3");

    std::vector<std::string> childPorts = link.GetStrConnections();
    std::string strMtxType = (mtxType == MtxType::LAMPORT) ? std::string(MTX_LAMPORT) : std::string(MTX_RA); 
    LOG_INFO("Launching processes\n");
    std::string p1; p1.append("LW.exe"); p1.append(" "); p1.append(name1); p1.append(" "); p1.append(strMtxType);p1.append(" ");
                    p1.append(childPorts[0]); p1.append(" "); p1.append(std::to_string(m_Id)); p1.append(" "); p1.append("2");

    std::string p2; p2.append("LW.exe"); p2.append(" "); p2.append(name2); p2.append(" "); p2.append(strMtxType);p2.append(" ");
                    p2.append(childPorts[1]); p2.append(" "); p2.append(std::to_string(m_Id)); p2.append(" "); p2.append("2"); p2.append(" "); p2.append(childPorts[0]);

    std::string p3; p3.append("LW.exe"); p3.append(" "); p3.append(name3); p3.append(" "); p3.append(strMtxType); p3.append(" ");
                    p3.append(childPorts[2]); p3.append(" "); p3.append(std::to_string(m_Id)); p3.append(" "); p3.append("2"); p3.append(" "); p3.append(childPorts[0]); p3.append(" "); p3.append(childPorts[1]);
    std::cout << p1 << std::endl;
    std::cout << p2 << std::endl;
    std::cout << p3 << std::endl;
    //              <name>    <mtx type>  <child server port>       <parent port(me)>                  <connection count>    <ports for the child to connect to>
    m_Processes[0].launch( p1.c_str() );
    m_Processes[1].launch( p2.c_str() );
    m_Processes[2].launch( p3.c_str() );
}

HWApp::~HWApp()
{
    delete m_Mutex;
}

void HWApp::run()
{
    m_Mutex->StartClientService(m_ParentId);
    
    std::string name(m_Name);
    name.append("\n");
    LOG_TRACE("Main app run\n");
    /* *** Child Connection startup *** */
    LOG_TRACE("Waiting for childs to be ready\n");
    WaitForChilds(3);   
    for (int i = 0; i < 5; i++)
    {
        m_Mutex->requestCS();                           // Request Token and wait for Tocken
        std::cout << name;
        //Sleep(100);
        LOG_TRACE("Notifying childs to start.\n");
        NotifyChildsToStart();                          // Send start to all childs
        LOG_TRACE("Waiting for childs to be ready\n");
        WaitForChilds(3);                               // Wait for all childs to finish
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

void HWApp::WaitForChilds(int count)
{
    std::unique_lock<std::mutex> lck(mtx_Childs);
    LOG_TRACE("Start wait.\n");
    cv_Childs.wait(lck, [&](){return m_ChildFinishes >= count;});
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
#include "MsgHandler.h"
#include "App.h"
#include "Log.h"

template<typename T>
static bool eraseFromVector(T toErase, std::vector<T>& vec)
{
    auto it = std::find(vec.begin(), vec.end(), toErase);
    if (it != vec.end())
    {
        vec.erase(it);
        return true;
    }   
    return false;
}


MsgHandler::MsgHandler(const Linker& comms)
 :  m_ParentId(comms.parentPort), m_Running(true), 
    m_Id(comms.serverPort), m_ChildFinishes(0), m_Begin(false)
{     
}

MsgHandler::~MsgHandler()
{
    LOG_TRACE("MsgHandler, closing all clients\n");
    for (int id : m_CurrentComms)
        App::RemoveClient(id);
    App::RemoveClient(m_ParentId);
    for (auto& thread : threads)
        thread->join();
    LOG_WARN("MsgHandler, End.\n");
}

/// @brief Single thread dedicated to each connected client
/// @param socket Actual client we are looking for
void MsgHandler::ClientService(int id, std::function<void(int, int, Tag)> handleMsg)
{
    bool connected = true;
    LOG_TRACE("ClientService started id: {}\n", id);
    while (connected)
    {
        switch (App::IncommingReadFrom(id))
        {
        case 1:     // Incomming read - > <Tag:1 byte(char)> <data: 4 bytes(int)>s
            LOG_TRACE("ClientService, incomming read from {}\n", id);
            std::array<char, 5> data;                           // Reception buffer
            App::ReceiveMsg(id, data);
            {
                std::lock_guard<std::mutex> lck(mtx_CallbackWait);
                handleMsg(*(int*)&data[1], id, (Tag)data[0]);   // Handle message callback
            }
            break;
        case 0:     // Reception closed
            connected = false;
            LOG_WARN("ClientService, Client disconnected, id: {}, my id: {}\n", id, m_Id);
            break;
        default:
            if (WSAGetLastError() != WSAETIMEDOUT)
            {
                LOG_ERROR("ClientService, recv failed with code {}, and id, {}, my id {}\n", WSAGetLastError(), id, m_Id);
                connected = false;
            }
            break;
        } 
    }
    LOG_WARN("ClientService, removing client.\n");
    App::RemoveClient(id);
}


void MsgHandler::BroadcastMsg(Tag tag, int msg)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    LOG_WARN("Broadcasting msg tag, {}\n", (char)tag);
    for (int i = 0; i < m_CurrentComms.size(); i++){
        LOG_TRACE("Sending msg to {}", m_CurrentComms[i]);
        App::SendMsg(m_CurrentComms[i], tag, msg); 
    }
}


void MsgHandler::AddClient(int id)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    m_CurrentComms.push_back(id);
}

void MsgHandler::StartClientService(int port)
{
    threads.push_back(new std::thread(&MsgHandler::ClientService, this, port, BIND_CALLBACK(MsgHandler::HandleMsg)));
}

void MsgHandler::eraseClient(int id)
{
    std::lock_guard<std::mutex> lock(mtx_DataLock);
    eraseFromVector(id, m_CurrentComms);
}

std::vector<std::string> Linker::GetProcessArgs(const std::string& exe, MtxType mtxType) const
{
    std::vector<std::string> arguments;
    std::string strMtxType = (mtxType == MtxType::LAMPORT) ? std::string(MTX_LAMPORT) : std::string(MTX_RA); 
    std::vector<std::string> childPorts = GetStrConnections();
    std::string connCount = std::to_string(connections.size()-1);
    std::string s;
    for (int i = 0; i < connections.size(); i++)
    {
        s.append(exe);s.append(" "); s.append("LW");s.append(std::to_string(i+1));s.append(" ");s.append(strMtxType); s.append(" ");
        s.append(childPorts[i]); s.append(" "); s.append(std::to_string(serverPort)); s.append(" "); s.append(connCount); 
        
        for (int e = 0; e < i; e++){
            s.append(" "); s.append(childPorts[e]);
        }
        arguments.push_back(s);
        s.clear();
    }
    return arguments;
}
#include "App.h"
#include "Utilities/Log.h"


App::App(const std::string& name, const Linker& link, MtxType mtxType)
    : m_Name(name), Server(link.serverPort), m_ClientsToLock(true), m_Mutex(nullptr), m_Id(link.serverPort)
{
    if (link.parentPort){
        AddClient(link.parentPort);
    }
}

App::~App()
{
    LOG_TRACE("Main app destructor.\n");
    delete m_Mutex;
}


void App::SendMsg(int desc, Tag tag, int msg)
{
    assertm(s_App->m_Sockets.find(desc) != s_App->m_Sockets.end(), "Send msg to unknown client!");
    std::array<char, 5> buffer;
    buffer[0] = (char)tag;
    memcpy_s(&buffer[1], sizeof(int), &msg, sizeof(int));
    s_App->m_Sockets[desc].Send(buffer);
}

int App::IncommingReadFrom(int id) 
{ 
    assertm(s_App->m_Sockets.find(id) != s_App->m_Sockets.end(), "Socket not found!"); 
    return s_App->m_Sockets[id].IncommingRead(); 
}

void App::RemoveClient(int id)
{
    if (s_App->m_Sockets.find(id) != s_App->m_Sockets.end()) 
        s_App->m_Sockets.erase(id);
    else LOG_WARN("Erasing non existent client!");
}

int App::AddClient(SOCKET sck)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    std::array<int,1> sckId;
    Socket::Receive(sck, sckId);
    assertm(sckId[0] != 0, "Wrong socket id!");
    m_Sockets.try_emplace(sckId[0], sck);
    LOG_TRACE("Connection added from {}, my id {}\n", sckId[0], m_Id);
    return sckId[0];
}

int App::AddClient(int port)
{
    std::lock_guard<std::mutex> lck(mtx_DataLock);
    m_Sockets.try_emplace(port, port);
    assertm(m_Sockets[port].Connected(), "Client failed to connect!");
    m_Sockets[port].Send(std::array<int,1>{m_Id});
    LOG_TRACE("Connected to {}, my id {}\n", port, m_Id);
    return port;
}

void App::Destroy()
{
    delete s_App; 
    Socket::Finit(); 
    Log::EndLogging();
}
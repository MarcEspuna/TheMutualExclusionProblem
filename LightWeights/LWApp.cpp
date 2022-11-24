#include "LWApp.h"
#include "CentMutex.h"
#include "LamportMutex.h"
#include "RAMutex.h"
#include "Log.h"
#include "io.h"

LWApp::LWApp(const std::string& name, const Linker& link,  MtxType mtxType)
    : m_Name(name), m_Mutex(nullptr)
{
    /* Init windows sockets */
    Log::CreateLogger(name);  
    Socket::Init();
    switch (mtxType)
    {
        case MtxType::LAMPORT:
            m_Mutex = (Lock*)new LamportMutex(link);
            break;
        case MtxType::RICART_AGRAWALA:
            m_Mutex = (Lock*)new RAMutex(link);    
            break;
        default:
            LOG_ERROR("Unsuported mutex type.\n");
            break;
    }
}

LWApp::~LWApp()
{
    delete m_Mutex;
    Socket::Finit();    // Finalize windows sockets
    Log::EndLogging();
}

void LWApp::run()
{
    LOG_INFO("Main app run\n");
    /* Not as leader */
    if (m_Mutex)
    {
        m_Name.append("\n");
        for (int i = 0; i < 8; i++)
        {
            m_Mutex->requestCS();
            Sleep(400);
            _write(1, m_Name.c_str(), (int)m_Name.size());
            m_Mutex->releaseCS();
        }
    } 
    LOG_INFO("Exit app run.\n")
}
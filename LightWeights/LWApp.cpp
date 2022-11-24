#include "LWApp.h"
#include "ProcessLauncher.h"
#include "LamportMutex.h"
#include "Log.h"
#include "io.h"

LWApp::LWApp(const std::string& name)
    : m_Name(name)
{
    /* Init windows sockets */
    Log::CreateLogger(name);  
    Socket::Init();
}

LWApp::~LWApp()
{
    Socket::Finit();    // Finalize windows sockets
}

void LWApp::run(Linker link)
{
    LOG_INFO("Main app run\n");
    /* Not as leader */
    LamportMutex laMutex(link);

    m_Name.append("\n");
    for (int i = 0; i < 8; i++)
    {
        laMutex.requestCS();
        Sleep(400);
        _write(1, m_Name.c_str(), (int)m_Name.size());
        laMutex.releaseCS();
    }
    
}
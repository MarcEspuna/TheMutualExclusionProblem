#include "HWApp.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
#include "Log.h"
#include "io.h"

HWApp::HWApp(const std::string& name)
    : m_Name(name)
{
    /* Init windows sockets */
    Log::CreateLogger(name);  
    Socket::Init();
}

HWApp::~HWApp()
{
    Socket::Finit();    // Finalize windows sockets
}

void HWApp::run(Linker link)
{
    LOG_INFO("Main app run");
    /* Not as leader */
    CentMutex centMutex(link, false);
    m_Name.append("\n");
    for (int i = 0; i < 5; i++)
    {
        Sleep(1000);
        centMutex.requestCS();
        _write(1, m_Name.c_str(), (int)m_Name.size());
        centMutex.releaseCS();
    }
    
}
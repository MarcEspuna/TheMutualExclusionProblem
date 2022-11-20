#include "HWApp.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
#include "Log.h"

HWApp::HWApp(const std::string& name)
    : m_Name(name)
{
    /* Init windows sockets */
    Socket::Init();
    Log::CreateLogger(name);  
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
    LOG_TRACE("Test");
    for (int i = 0; i < 5; i++)
    {
        Sleep(1000);
        centMutex.requestCS();
        std::cout << "Heavy weight "<< m_Name << "\n";
        centMutex.releaseCS();
    }
    
}
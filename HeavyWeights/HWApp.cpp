#include "HWApp.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"

HWApp::HWApp(const std::string& name)
    : m_Name(name)
{
    /* Init windows sockets */
    Socket::Init();  
}

HWApp::~HWApp()
{
    Socket::Finit();    // Finalize windows sockets
}

void HWApp::run(Linker link)
{
    CentMutex centMutex(link, false);

    for (int i = 0; i < 5; i++)
    {
        Sleep(1000);
        centMutex.requestCS();
        std::cout << "Heavy weight "<< m_Name << "\n";
        centMutex.releaseCS();
    }
    
}
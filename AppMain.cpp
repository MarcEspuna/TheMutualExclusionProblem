#include "AppMain.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
//#define ACTIVE_LOGGING
#include "Log.h"

AppMain::AppMain(const std::string& name, const Linker& link, MtxType mtxType)
    : App(name, link, mtxType)
{
    /* Create centralized mutex  */
    m_Mutex = Lock::Create<CentMutex>({8250, 0, 0, {}}, true);
}

AppMain::~AppMain()
{

}

void AppMain::run()
{
    LOG_INFO("Starting HW_A");
    Process Hw_A("HW.exe");
    Hw_A.launch("HW.exe 8250 8888 HEAVY-WEIGHT-A " MTX_LAMPORT " 8700 8750 8800");

    LOG_INFO("Starting HW_B");
    Process Hw_B("HW.exe");
    Hw_B.launch("HW.exe 8250 8889 HEAVY-WEIGHT-B " MTX_RA " 8600 8610 8620");

    //LOG_INFO("Starting HW_B");
    //Process Hw_C("HW.exe");
    //Hw_C.launch("HW.exe 8250 8890 HEAVY-WEIGHT-C " MTX_LAMPORT " 8630 8640 8650");

    LOG_TRACE("Waiting heavy weights\n");
    Hw_A.wait();
    Hw_B.wait();
    //Hw_C.wait();
    std::cout << "All processes finished, press enter to finish.\n";
    std::cin.get();
}


void AppMain::IncommingConnection(SOCKET client)
{
    LOG_TRACE("Adding connection to mutex!");
    int clientId = AddClient(client);
    m_Mutex->AddClient(clientId);
    m_Mutex->StartClientService(clientId);   
}
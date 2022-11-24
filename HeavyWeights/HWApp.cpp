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
    Log::EndLogging();
}

void HWApp::run(Linker link, const std::vector<std::string>& childPorts, const char* mtxType)
{
    LOG_INFO("Main app run\n");
    /* Not as leader */
    CentMutex centMutex(link, false);

    centMutex.requestCS();
    Process LW_1("LW.exe");
    Process LW_2("LW.exe");
    Process LW_3("LW.exe");

    std::string name1 = m_Name;
    std::string name2 = m_Name;
    std::string name3 = m_Name;

    name1.append("-LW_1");
    name2.append("-LW_2");
    name3.append("-LW_3");

    LW_1.launch({name1.c_str(), mtxType, childPorts[0].c_str(), std::to_string(link.serverPort).c_str()});
    LW_2.launch({name2.c_str(), mtxType, childPorts[1].c_str(), std::to_string(link.serverPort).c_str(), childPorts[0].c_str()});
    LW_3.launch({name3.c_str(), mtxType, childPorts[2].c_str(), std::to_string(link.serverPort).c_str(), childPorts[0].c_str(), childPorts[1].c_str()});

    m_Name.append("\n");
    _write(1, m_Name.c_str(), (int)m_Name.size());

    LOG_INFO("Waiting child processes.\n");
    LW_1.wait();
    LW_2.wait();
    LW_3.wait();
    LOG_INFO("Realeasing CS to leader.\n");
    centMutex.releaseCS();
}
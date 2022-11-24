#include "App.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"
//#define ACTIVE_LOGGING
#include "Log.h"

App::App()
{
    /* Init windows sockets */
    Log::CreateLogger("CORE");
    Socket::Init();  
}

App::~App()
{
    Socket::Finit();    // Finalize windows sockets
    Log::EndLogging();
}

void App::run()
{
    LOG_INFO("Running main app\n");
    Linker link = {8250, 0, {}};
    CentMutex centMutex(link, true);

    LOG_INFO("Starting HW_A");
    Process Hw_A("HW.exe");
    Hw_A.launch({"8888", "8250", "HEAVY-WEIGHT-A", "8700", "8750", "8800"});

    LOG_INFO("Starting HW_B");
    Process Hw_B("HW.exe");
    Hw_B.launch({"8889", "8250", "HEAVY-WEIGHT-B", "9090", "9091", "9092"});

    Hw_A.wait();
    Hw_B.wait();
    std::cin.get();
}
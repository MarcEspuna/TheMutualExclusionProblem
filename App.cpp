#include "App.h"
#include "ProcessLauncher.h"
#include "CentMutex.h"

App::App()
{
    /* Init windows sockets */
    Socket::Init();  
}

App::~App()
{
    Socket::Finit();    // Finalize windows sockets
}

void App::run()
{
    Linker link = {8250, 0, {}};
    CentMutex centMutex(link, true);

    std::cout << "[APP]: Starting HW_A:\n";
    Process Hw_A("HW.exe");
    Hw_A.launch({"8888", "8250", "Heavy-weight-A"});

    std::cout << "[APP]: Starting HW_B\n";
    Process Hw_B("HW.exe");
    Hw_B.launch({"8889", "8250", "Heavy-weight-B"});

    Hw_A.wait();
    Hw_B.wait();
}
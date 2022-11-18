#include "App.h"
#include "Utilities/Process.h"

App::App()
{

}

App::~App()
{

}

void App::run()
{
    std::cout << "This is a test\n";
    Process process("HW.exe");
    process.launch({"passedFromApp"});
    process.wait();
    
}
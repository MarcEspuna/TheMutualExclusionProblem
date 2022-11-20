#include <iostream>
#include <crtdbg.h>
#include "App.h"
#include "ProcessLauncher.h"

int main(int argc, char** argv)
{   
    /* Memory leak check when running debug */
    #ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    #endif
    App app;
    app.run();
    
    return 0;
}
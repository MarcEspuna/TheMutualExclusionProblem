#include <iostream>
#include <crtdbg.h>
#include "Commons.h"
#include "Socket.h"
#include "AppMain.h"
#include "ProcessLauncher.h"
#include "Log.h"

int main(int argc, char** argv)
{   
    /* Memory leak check when running debug */
    #ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    #endif

    App::Create<AppMain>("CORE", {8250, 0, 0, {}}, MtxType::CENT_MUTEX);
    App::Run();
    App::Destroy();
    
    return 0;
}
#include <iostream>
#include "Commons.h"
#include "CentMutex.h"
#include "ProcessLauncher.h"
#include "HWApp.h"
#include "App.h"

//char* argv[] = { "nothing", "8250", "8888","HEAVY-WEIGHT-A", MTX_LAMPORT, "8700", "8750", "8800" };

/* Args: <exe> <parent port> <own port> <name> <mtx type> <child ports>*/
int main(int argc, char** argv)
{   
    //int argc = 8;
    Linker link;
    MtxType mtxType;
    if (argc > 4)
    {
        link.parentPort = atoi(argv[1]);
        link.serverPort = atoi(argv[2]);
        mtxType = (strcmp(argv[4], MTX_LAMPORT) == 0) ? MtxType::LAMPORT : MtxType::RICART_AGRAWALA; 
        for (int i = 5; i < argc; i++)
            link.connections.push_back(atoi(argv[i]));
    }

    App::Create<HWApp>({argv[3]}, link, mtxType);
    HWApp::Run();
    HWApp::Destroy();
    
    return 0;
}
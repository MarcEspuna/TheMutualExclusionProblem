#include "Commons.h"
#include "LWApp.h"

/// <exe> <name> <mtx type> <server port> <parent port> <total connections> <ports to connect to>
int main(int argc, char** argv)
{
    Linker link;
    if (argc > 5)
    {
        MtxType mtxType = (strcmp(argv[2], MTX_LAMPORT) == 0) ? MtxType::LAMPORT : MtxType::RICART_AGRAWALA; 
        link.serverPort = atoi(argv[3]);
        link.parentPort = atoi(argv[4]);
        link.totalConnections = atoi(argv[5]);
        
        for (int i = 6; i < argc; i++)
            link.connections.push_back(atoi(argv[i]));

        App::Create<LWApp>(argv[1], link, mtxType);
        App::Run();
        App::Destroy();     
    }

    return 0;
}
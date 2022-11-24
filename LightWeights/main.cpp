#include "Commons.h"
#include "LWApp.h"

/// <exe> <name> <mtx type> <server port> <parent port> <ports to connect to>
int main(int argc, char** argv)
{
    Linker link;
    if (argc > 4)
    {
        MtxType mtxType = (strcmp(argv[2], MTX_LAMPORT) == 0) ? MtxType::LAMPORT : MtxType::RICART_AGRAWALA; 
        link.serverPort = atoi(argv[3]);
        link.parentPort = atoi(argv[4]);

        for (int i = 5; i < argc; i++)
            link.connections.push_back(atoi(argv[i]));

        LWApp app(argv[1], link, mtxType);    
        app.run();
    }
    return 0;
}
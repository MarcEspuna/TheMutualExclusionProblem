#include <iostream>
#include "LWApp.h"

/// <name> <server port> <parent port> <ports to connect to>
int main(int argc, char** argv)
{
    Linker link;
    if (argc > 3)
    {
            link.serverPort = atoi(argv[2]);
            link.parentPort = atoi(argv[3]);
        for (int i = 4; i < argc; i++)
            link.connections.push_back(atoi(argv[i]));

        LWApp app(argv[1]);    
        app.run(link);
    }
    return 0;
}
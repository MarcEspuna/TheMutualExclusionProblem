#include <iostream>
#include "Commons.h"
#include "CentMutex.h"
#include "ProcessLauncher.h"
#include "HWApp.h"

/* Args: <exe> <parent port> <own port> <name> <mtx type> <child ports>*/
int main(int argc, char** argv)
{   
    std::vector<std::string> childPorts;
    Linker link;
    if (argc > 4)
    {
        link.serverPort = atoi(argv[1]);
        link.parentPort = atoi(argv[2]);
        for (int i = 5; i < argc; i++)
            childPorts.push_back(argv[i]);
    }
    HWApp app({argv[3]});
    app.run(link, childPorts, argv[4]);
    return 0;
}
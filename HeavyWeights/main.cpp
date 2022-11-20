#include <iostream>
#include "Commons.h"
#include "CentMutex.h"
#include "ProcessLauncher.h"
#include "HWApp.h"

/* Args: <exe> <parent port> <own port> <name>*/
int main(int argc, char** argv)
{   
    Linker link;
    if (argc > 3)
    {
        link.serverPort = atoi(argv[1]);
        link.parentPort = atoi(argv[2]);
    }
    HWApp app({argv[3]});
    app.run(link);
    return 0;
}
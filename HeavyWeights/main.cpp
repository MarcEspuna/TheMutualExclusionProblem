#include <iostream>
#include "CentMutex.h"
#include "Process.h"
#include "Commons.h"

/* Args: <exe> <parent port> <own port> <name>*/
int main(int argc, char** argv)
{   
    Linker link;
    if (argc > 3)
    {
        link.parentPort = atoi(argv[1]);
        link.serverPort = atoi(argv[2]);
    }
    
    CentMutex centMutex(link, false);
    for (int i = 0; i < 5; i++)
    {
        Sleep(1000);
        centMutex.requestCS();
        std::cout << "Heavy weight "<< argv[3] << "\n";
        centMutex.releaseCS();
    }
    return 0;
}
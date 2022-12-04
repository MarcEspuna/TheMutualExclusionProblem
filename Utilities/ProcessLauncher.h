#pragma once
#include "Commons.h"

class Process {
public:
    Process();
    ~Process();

    void wait();
    
    int launch(const char* args) {
        char* strArgs = new char[strlen(args)+1];
        strcpy_s(strArgs,strlen(args)+1, args);

        arguments.push_back(strArgs);
        int err = CreateProcess(NULL, arguments.back(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
        
        if (err)    return 1;
        return 0;
    }   

private:
    STARTUPINFO info = {sizeof(info)};
    PROCESS_INFORMATION processInfo;
    std::vector<LPTSTR> arguments;

};
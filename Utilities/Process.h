#pragma once
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <initializer_list>

class Process {
public:
    Process(const char* execPath);
    ~Process();

    void wait();
    
    template<class T>
    int launch(std::initializer_list<T> list) {
        char* strArgs = new char[strlen(path)+1];
        strcpy_s(strArgs, strlen(path)+1, path);
        
        for (const auto& arg : list)
        {
            int size = (int)strlen(strArgs)+(int)strlen(arg)+2;
            strArgs = (char*)realloc(strArgs, size);
            sprintf_s(strArgs, size,"%s %s", strArgs, arg);
        }

        LPTSTR cmdArgs = strArgs;
        int err = CreateProcess(path, cmdArgs, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
        delete cmdArgs;
        if (err)    return 1;
        return 0;
    }   

private:
    char* path;                             // Executable path
    STARTUPINFO info={sizeof(info)};
    PROCESS_INFORMATION processInfo;

};
#include "ProcessLauncher.h"


Process::Process(const char* execPath)
: info({sizeof(info)})
{
    path = new char[strlen(execPath)+1];
    strcpy_s(path, strlen(execPath)+1, execPath);

    ZeroMemory( &info, sizeof(info) );
    info.cb = sizeof(info);
    ZeroMemory( &processInfo, sizeof(processInfo) );
}

Process::~Process()
{
    delete path;
    for (auto arg : arguments)
        delete arg;
}

void Process::wait()
{
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}
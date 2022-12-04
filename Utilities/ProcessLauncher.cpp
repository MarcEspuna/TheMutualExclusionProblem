#include "ProcessLauncher.h"


Process::Process()
: info({sizeof(info)})
{
    ZeroMemory( &info, sizeof(info) );
    info.cb = sizeof(info);
    ZeroMemory( &processInfo, sizeof(processInfo) );
}

Process::~Process()
{
    for (auto arg : arguments)
        delete arg;
}

void Process::wait()
{
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}
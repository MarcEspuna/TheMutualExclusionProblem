#include "Process.h"


Process::Process(const char* execPath)
: info({sizeof(info)})
{
    path = new char[strlen(execPath)+1];
    strcpy_s(path, strlen(execPath)+1, execPath);
}

Process::~Process()
{
    delete path;
}

void Process::wait()
{
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
}
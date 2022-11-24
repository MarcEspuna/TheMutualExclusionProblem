#pragma once
#include "Commons.h"
#include "Lock.h"

class LWApp {
public:
    LWApp(const std::string& name, const Linker& link, MtxType mtxType);
    ~LWApp();

    void run();
private:
    std::string m_Name;
    Lock* m_Mutex;
};
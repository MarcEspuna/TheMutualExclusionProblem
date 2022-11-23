#pragma once
#include "Commons.h"
#include "CentMutex.h"

class LWApp {
public:
    LWApp(const std::string& name);
    ~LWApp();

    void run(Linker link);
private:
    std::string m_Name;
};
#pragma once
#include "Commons.h"
#include "Server.h"
#include "Lock.h"
#include "App.h"

class LWApp : public App {
public:

private:
    LWApp(const std::string& name, const Linker& link, MtxType mtxType);
    ~LWApp();
    
    friend class App;
private:
    void IncommingConnection(SOCKET client) override;
    void run();

    Linker test;
};
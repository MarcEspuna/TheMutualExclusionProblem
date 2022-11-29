#include "Commons.h"
#include "Server.h"
#include "Lock.h"
#include "App.h"

class AppMain : public App {
public:

protected:
    ~AppMain();
    AppMain(const std::string& name, const Linker& link, MtxType mtxType);

    void run() override;
    void IncommingConnection(SOCKET client) override;
    friend class App;
private:

};
#include "Commons.h"
#include "CentMutex.h"

class HWApp {
public:
    HWApp(const std::string& name);
    ~HWApp();

    void run(Linker link, const std::vector<std::string>& childPorts, const char* mtxType);
private:
    std::string m_Name;
};
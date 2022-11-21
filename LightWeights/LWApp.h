#include "Commons.h"
#include "CentMutex.h"

class HWApp {
public:
    HWApp(const std::string& name);
    ~HWApp();

    void run(Linker link);
private:
    std::string m_Name;
};
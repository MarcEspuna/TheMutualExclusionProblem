#include "Commons.h"
#include "CentMutex.h"
#include "Client.h"
#include "ProcessLauncher.h"
#include "App.h"

class HWApp : public App {
public:

protected:
    void run() override;
    void IncommingConnection(SOCKET client) override;

    HWApp(const std::string& name, const Linker& link, MtxType mtxType);
    ~HWApp();
private:
    std::vector<Process> m_Processes;               // Light weight processes to launch

    int m_ChildFinishes;

    std::mutex mtx_Childs;
    std::condition_variable cv_Childs;

    friend class App;
private: 
    void WaitForChilds(int count);
    void ChildReadyNotify(int id);
    void NotifyChildsToStart();
    void BroadcastMsgToChilds(Tag tag, int msg = 0);
};
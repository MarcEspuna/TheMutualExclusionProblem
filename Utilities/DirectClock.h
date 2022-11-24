#pragma once
#include "Commons.h"

class DirectClock{
public:
    DirectClock(int myId)
        : m_Id(myId), m_Ticks({})
    {
        m_Ticks[m_Id] = 1;
    }
    ~DirectClock() = default;

    inline int GetValue(int i) { return m_Ticks[i]; }
    inline void Tick() { m_Ticks[m_Id]++; }
    inline void SendAction() { Tick(); }
    
    void RecieveAction(int sender, int sentValue) {
        m_Ticks[sender] = (m_Ticks[sender] > sentValue ? m_Ticks[sender] : sentValue);
        m_Ticks[m_Id] = (m_Ticks[m_Id] > sentValue ? m_Ticks[m_Id] : sentValue) + 1;
    }

    std::unordered_map<int,int>::iterator begin() { return m_Ticks.begin(); }
    std::unordered_map<int,int>::iterator end() { return m_Ticks.end(); }

private:
    std::unordered_map<int, int> m_Ticks;      // <Id, ticks>
    int m_Id;
};
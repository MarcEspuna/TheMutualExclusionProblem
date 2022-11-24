#pragma once

class LamportClock {
public:
    LamportClock() : m_Timestamp(1) {}
    ~LamportClock() = default;

    inline int GetValue() {return m_Timestamp; }
    inline void Tick() { m_Timestamp++;}
    inline void SendAction() {Tick();}
    inline void ReceiveAction(int src, int timestamp) {m_Timestamp = (m_Timestamp < timestamp) ? timestamp : m_Timestamp;}

private:
    int m_Timestamp;

};
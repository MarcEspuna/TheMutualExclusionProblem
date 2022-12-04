#pragma once
#include "Commons.h"
#include "Log.h"

enum SocketType {
    SERVER, CLIENT, UNKNOWN
};

#define OWN_SOCKET  0
class Socket {
public:
    Socket();
    Socket(const Socket& copy) = delete;
    Socket(SOCKET socket, const sockaddr_in& details);

    virtual ~Socket();                          // Important to declare virtual destructor! 

    /* Transmition */
    template<typename T,size_t S>
    void Send(const std::array<T, S> segment) const;

    template<typename T, size_t S>
    int Receive(std::array<T, S>& buffer);

    template<typename T, size_t S>
    static int Receive(SOCKET s, std::array<T, S>& buffer);
    
    template<typename T,size_t S>
    static void Send(SOCKET s, const std::array<T, S> segment);

    inline SOCKET getDescriptor() const { return s; }
    inline sockaddr_in getDetails() const { return server;}

    inline bool Connected() {return m_Connected; };

    int IncommingRead();

    /* Initialization and finalization */
    static void Init();                 // Windows sockets initialization
    static void Finit();                // Finalization of windows sockets

    void close();
    void gracefulClose() const;

    inline bool operator==(const Socket& other) { return s == other.s; }

protected:
	SOCKET s;							// Actual socket pointer
	sockaddr_in server;					// Server data

    bool m_Connected;

    /* Function pointer that gets called every time we receive anything from the client */
	void(*r_Callback)(Socket);

	bool InitSocket();                  // New socket creation/Initialization
};

/* Transmition Implementations */
template<typename T,size_t S>
void Socket::Send(const std::array<T, S> segment) const
{
    if (send(s, (char*)segment.data(), sizeof(T)*S, 0) == SOCKET_ERROR)
        LOG_ASSERT(false, "Error on send! Error code {}", WSAGetLastError());
}
template<typename T, size_t S>
int Socket::Receive(std::array<T, S>& buffer)
{
    return recv(s, (char*)buffer.data(), sizeof(T)*S, 0);
}

template<typename T, size_t S>
static int Socket::Receive(SOCKET s, std::array<T, S>& buffer)
{
    return recv(s, (char*)buffer.data(), sizeof(T)*S, 0);
}

template<typename T,size_t S>
static void Socket::Send(SOCKET s, const std::array<T, S> segment)
{
    send(s, (char*)segment.data(), sizeof(T)*S, 0);
}
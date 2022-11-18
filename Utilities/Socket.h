#pragma once
#include "Commons.h"
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <array>

enum SocketType {
    SERVER, CLIENT, UNKNOWN
};

#define OWN_SOCKET  0
class Socket {
public:
    Socket();
    Socket(const Socket& copy);

    Socket(SOCKET socket, const sockaddr_in& details);

    virtual ~Socket();                          // Important to declare virtual destructor! 

    /* Transmition */
    template<typename T,size_t S>
    void Send(const std::array<T, S> segment)
    {
        send(s, (char*)segment.data(), sizeof(T)*S, 0);
    }
    template<typename T, size_t S>
    int Receive(std::array<T, S>& buffer)
    {
        return recv(s, (char*)buffer.data(), sizeof(T)*S, 0);
    }

    inline SOCKET getDescriptor() const { return s; }
    inline sockaddr_in getDetails() const { return server;}

    void close() const;
    void gracefulClose() const;

    /* Initialization and finalization */
    static void Init();                 // Windows sockets initialization
    static void Finit();                // Finalization of windows sockets

    inline bool operator==(const Socket& other) { return s == other.s; }

protected:
	SOCKET s;							// Actual socket pointer
	sockaddr_in server;					// Server data

    /* Function pointer that gets called every time we receive anything from the client */
	void(*r_Callback)(Socket);

	bool InitSocket();                  // New socket creation/Initialization
};
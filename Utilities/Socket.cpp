#include "Socket.h"
#include <basetsd.h>
#include <mutex>
#include <stdint.h>
#include <winsock2.h>
#include "Log.h"

Socket::Socket()
{
    InitSocket();
}

Socket::Socket(const Socket& copy)
	: s(copy.s), server(copy.server) {}

Socket::Socket(SOCKET socket, const sockaddr_in& details)
	: s(socket), server(details)
{}

Socket::~Socket()
{
	closesocket(s);
}

bool Socket::InitSocket()
{
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		LOG_ERROR("[INIT SOCKET]: Could not create socket : {}", WSAGetLastError());
		return false;
	}
	LOG_INFO("Socket {} initialized.", s);
	return true;
}

void Socket::Init()
{
	WSADATA wsa;						// Winsocket
	LOG_INFO("Initialising Winsock Client...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		LOG_ERROR("Failed. Error Code : {}", WSAGetLastError());
	}
}

void Socket::Finit()
{
	WSACleanup();
	LOG_WARN("Finalization of windows sockets.");
}

void Socket::close() const
{
	closesocket(s);
	LOG_WARN("Closed socket, {}", s);
}

void Socket::gracefulClose() const
{
	shutdown(s, SD_SEND);
}
#include "Socket.h"
#include <basetsd.h>
#include <mutex>
#include <stdint.h>
#include <winsock2.h>
#include "Log.h"

Socket::Socket()
	: m_Connected(false)
{
    InitSocket();
}

Socket::Socket(SOCKET socket, const sockaddr_in& details)
	: s(socket), server(details)
{}

Socket::~Socket()
{
	if (m_Connected)
	{
		gracefulClose();
		close();
		m_Connected = false;
	}
}

bool Socket::InitSocket()
{
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		LOG_ERROR("[INIT SOCKET]: Could not create socket : {}\n", WSAGetLastError());
		return false;
	}
	LOG_INFO("Socket {} initialized.\n", s);
	return true;
}

void Socket::Init()
{
	WSADATA wsa;						// Winsocket
	LOG_INFO("Initialising Winsock Client...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		LOG_ERROR("Failed. Error Code : {}", WSAGetLastError());
	}
}

void Socket::Finit()
{
	WSACleanup();
	LOG_WARN("Finalization of windows sockets.\n");
}

void Socket::close()
{
	if (m_Connected)
	{
		closesocket(s);
		m_Connected = false;
		LOG_WARN("Closed socket, {}\n", s);
	}
}

void Socket::gracefulClose() const
{
	shutdown(s, SD_SEND);
}
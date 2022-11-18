#include "Socket.h"
#include <basetsd.h>
#include <mutex>
#include <stdint.h>
#include <winsock2.h>

Socket::Socket()
{
    InitSocket();
}

Socket::Socket(const Socket& copy)
	: s(copy.s), server(copy.server) { std::cout << "Socket memory copied\n"; }

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
		std::cout << "[INIT SOCKET]: Could not create socket : " << WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "[SOCKET]: Socket initialized." << std::endl;
	return true;
}

void Socket::Init()
{
	WSADATA wsa;						// Winsocket
	std::cout << "[INIT WINSOCK]: Initialising Winsock Client...\n";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "[INIT WINSOCK]: Failed. Error Code : " << WSAGetLastError() << std::endl;
	}
}

void Socket::Finit()
{
	WSACleanup();
	std::cout << "[FINI WINSOCK]: Finalization of windows sockets." << std::endl;
}

void Socket::close() const
{
	closesocket(s);
	std::cout << "Closed socket: " << s << std::endl;
}

void Socket::gracefulClose() const
{
	shutdown(s, SD_SEND);
}
#include "Client.h"
#include "Commons.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Log.h"

Client::Client() : m_Connected(false) {}

Client::Client(int port)
	:m_Connected(false)
{
	LOG_INFO("Initialized client socket.\n");
	Connect(port);
}

Client::Client(SOCKET socket, sockaddr_in details)
	: Socket(socket, details), m_Connected(false)
{	
}

Client::~Client() {
	LOG_WARN("Client closed, {}\n", s);
}

void Client::Connect(const unsigned int& port, const char* address)
{
	server.sin_port = htons(port);
	//server.sin_addr.s_addr = inet_addr(address);
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, address, &server.sin_addr.s_addr) <= 0)
	{
		LOG_ERROR("Client, Error configuring IP");
		return;
	}

	//Connect to remote server
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		LOG_ERROR("Client, error connecting to server with code: {}\n", WSAGetLastError());
	}
	else{
		LOG_INFO("Client, successfully connected to server\n");
		m_Connected = true;
	}
}


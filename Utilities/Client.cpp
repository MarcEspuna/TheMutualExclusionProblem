#include "Client.h"
#include "Commons.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Log.h"

Client::Client() {}

Client::Client(int port)
{
	Connect(port);
}

Client::Client(SOCKET socket)
	: Client(socket, {})
{ }

Client::Client(SOCKET socket, sockaddr_in details)
	: Socket(socket, details)
{	
	if (socket != 0) m_Connected = true;
	else 			assertm(false, "Creating invalid socket!");
}

Client::~Client() { }

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
		LOG_ERROR("Client, error connecting to server with code: {}, port {}\n", WSAGetLastError(), port);
	}
	else{
		assertm(m_Connected == false, "This client is already connected!");
		LOG_INFO("Client, successfully connected to server, port {}\n", port);
		m_Connected = true;
	}
}


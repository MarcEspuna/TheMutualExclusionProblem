#include "Client.h"
#include "Commons.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

Client::Client() : m_Connected(false) {}

Client::Client(int port)
	:m_Connected(false)
{
	std::cout << "[CLIENT]: Initialized client socket." << std::endl;
	Connect(port);
}

Client::Client(SOCKET socket, sockaddr_in details)
	: Socket(socket, details), m_Connected(false)
{	
}

Client::~Client() {

	std::cout << "[CLIENT]: Client closed, " << s << std::endl;
}

void Client::Connect(const unsigned int& port, const char* address)
{
	server.sin_port = htons(port);
	//server.sin_addr.s_addr = inet_addr(address);
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, address, &server.sin_addr.s_addr) <= 0)
	{
		std::cout << "[CLIENT]: Error configuring IP\n" << std::endl;
		exit(0);
		return;
	}

	//Connect to remote server
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		std::cout << "[CLIENT ERROR]: Error connecting to server" << std::endl;
		std::cout << "Error code: " << WSAGetLastError() << std::endl; 
	}
	else{
		std::cout << "[CLIENT]: Successfully connected to server" << std::endl;
		m_Connected = true;
	}
}


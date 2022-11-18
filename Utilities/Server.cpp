#include "Server.h"
#include <iostream>

Server::Server() {}

Server::Server(unsigned int port)
{
	std::cout << "[SERVER]: Server socket initialized." << std::endl;
	Bind(port);	
}

Server::~Server()
{
	std::cout << "[SERVER]: Socket closed, " << s << std::endl;
}

SOCKET Server::acceptClient() const
{
	sockaddr_in cl;						// Struct used for client socket address
	int c = sizeof(struct sockaddr_in);
	SOCKET new_socket = accept(s, (struct sockaddr*)&cl, &c);
	if (new_socket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		switch (error)
		{
		case WSAEINTR:
			std::cout << "[SERVER]: Interruption function call, stopping listening for connections..." << std::endl;
			break;
		default:
			std::cout << "[SERVER]: Accept failed with error code " << error << std::endl;
			break;
		}
		return 0;
	}
	std::cout << "[SERVER]: Connection accepted" << std::endl;
	return new_socket;
}

void Server::Bind(const unsigned int& port, const unsigned long& address)
{
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = address;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "[SERVER ERROR]: Bind failed with error code : " << WSAGetLastError() << std::endl;
	}
	std::cout << "[SERVER]: Bind done.\n";
}


void Server::listenConn(const unsigned int& connectionCount) const
{
	listen(s, connectionCount);
	std::cout << "[SERVER]: Listening..." << std::endl;
}
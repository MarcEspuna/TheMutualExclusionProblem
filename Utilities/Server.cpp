#include "Server.h"
#include <iostream>
#include "Log.h"

Server::Server() {}

Server::Server(unsigned int port)
{
	LOG_INFO("[SERVER]: Server socket initialized.");
	Bind(port);	
}

Server::~Server()
{
	LOG_WARN("Server, Socket closed, {}", s);
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
			LOG_WARN("Server, Interruption function call, stopping listening for connections...");
			break;
		default:
			LOG_ERROR("Server, Accept failed with error code: {} ", error);
			break;
		}
		return 0;
	}
	LOG_INFO("Server, connection accepted.");
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
		LOG_ERROR("[SERVER ERROR]: Bind failed with error code : {}", WSAGetLastError());
		return;
	}
	LOG_INFO("[SERVER]: Bind done.");
}


void Server::listenConn(const unsigned int& connectionCount) const
{
	listen(s, connectionCount);
}
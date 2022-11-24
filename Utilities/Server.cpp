#include "Server.h"
#include <iostream>
#include "Log.h"

Server::Server() {}

Server::Server(unsigned int port)
	: m_Port(port)
{
	LOG_INFO("Server, Server socket initialized.\n");
	Bind(port);	
}

Server::~Server()
{}

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
			LOG_WARN("Server, Interruption function call, stopping listening for connections...\n");
			break;
		default:
			LOG_ERROR("Server, Accept failed with error code: {}\n", error);
			break;
		}
		return 0;
	}
	LOG_INFO("Server, connection accepted.\n");
	return new_socket;
}

void Server::Bind(const unsigned int& port, const unsigned long& address)
{
	m_Port = port;
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = address;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		LOG_ERROR("Server, bind failed, port {}, error code : {}\n", port, WSAGetLastError());
		return;
	}
	m_Connected = true;
	LOG_INFO("Server, Bind done.\n");
}


void Server::listenConn(const unsigned int& connectionCount) const
{
	listen(s, connectionCount);
}
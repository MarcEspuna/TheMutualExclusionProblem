#pragma once
#include "Socket.h"
#include "Commons.h"

class Client : public Socket
{
public:
	/* Only initializes */
	Client();
	/* Directly connects to server */
	Client(int port);
	/* Directly specify attributes */
	Client(SOCKET socket);
	/* Directly specify attributes */
	Client(SOCKET socket, sockaddr_in details);

	virtual ~Client();

	void Connect(const unsigned int& port, const char* address = "127.0.0.1");

};


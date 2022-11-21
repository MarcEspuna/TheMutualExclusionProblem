#pragma once
#include "Commons.h"
#include "Socket.h"
#include "Client.h"

class Server : public Socket
{
public:
	Server();
	Server(unsigned int port);
	virtual ~Server();

	/* Getters */
	inline int GetPort() { return m_Port; }
	
	/* Accept incomming connection */
	SOCKET acceptClient() const;
	/* Listen to incomming client connections */
	void listenConn(const unsigned int& connections = 1) const;

	/* Bind server to specific port and address(localhost if no address is specifed) */
	void Bind(const unsigned int& port, const unsigned long& address = INADDR_ANY);
private:
	int m_Port;

};


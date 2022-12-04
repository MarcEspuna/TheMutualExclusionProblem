#pragma once
#include "Commons.h"
#include "Socket.h"
#include "Client.h"

class Server : public Socket
{
public:
	Server();
	Server(int port);
	virtual ~Server();

	/* Getters */
	inline int GetPort() { return m_Port; }
	
	/* Accept incomming connection */
	SOCKET AcceptClient() const;
	/* Listen to incomming client connections */
	void ListenConn(const unsigned int& connections = 1) const;

	/* Bind server to specific port and address(localhost if no address is specifed) */
	void Bind(const unsigned int& port, const unsigned long& address = INADDR_ANY);

	/* Start dispatching incomming connections */
	void StartConnectionHandling();

protected:
	virtual void IncommingConnection(SOCKET client) = 0;				// Incomming connection callback
private:
	int m_Port;															// Server port
	std::thread* m_Conectivity;											// Thread used to handle incomming connections from clients 
	std::function<void(SOCKET)> m_Callback;								// Funcion that gets called when a client is connecting to the server
private:
	void IncommingConnectionsHandler();									// Function that handles incomming connections on the server (used by m_Conectivity thread)

};


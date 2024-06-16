#include <iostream>
#include "TCPServer.h"

int main()
{
	TCPServer tcpServer;
	if (!tcpServer.init("127.0.0.1", 8888))
	{
		std::cout << "tcpClient init failed" << std::endl;
		return 0;
	}
	tcpServer.start();

	return 0;
}
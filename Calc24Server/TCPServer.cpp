#include "TCPServer.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>



#include <iostream>

#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game\n"

bool TCPServer::init(const std::string& ip, uint16_t port)
{

	srand(time(nullptr));

	//1.创建一个侦听socket
	m_listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenfd == -1)
	{
		std::cout << "create listen socket error." << std::endl;
		return false;
	}
	//端口号复用
	int optval = 1;
	//TODO: 判断函数是否调用成功
	::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
	::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));


	//2.初始化服务器地址
	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindaddr.sin_port = htons(port);
	if (::bind(m_listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
	{
		std::cout << "bind listen socket error." << std::endl;
		return false;
	}

	

	//3.启动侦听
	if (::listen(m_listenfd, SOMAXCONN) == -1)
	{
		std::cout << "listen error." << std::endl;
		return false;
	}

	return true;
}

void TCPServer::start()
{
	while (true)
	{
		struct sockaddr_in clientaddr;
		socklen_t clientaddrlen = sizeof(clientaddr);
		//4. 接受客户端连接
		int clientfd = ::accept(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientfd == -1)
		{
			std::cout << "accept error" << std::endl;
			return;
		}

		auto spThread = std::make_shared<std::thread>(TCPServer::clientThreadFunc,this,clientfd);
		
		//m_clientfdToThread[clientfd] = spThread;  考虑这种方法 效率上不是最高的， 应该会生成一份thread 的拷贝
		m_clientfdToThread[clientfd] = std::move(spThread);

		
	}


}

void TCPServer::clientThreadFunc(int clientfd)
{

	while (true)
	{
		std::cout << "new client connected: " << std::endl;

		if (!sendWelcomeMsg(clientfd))
		{
			::close(clientfd);
			return;
		}
	

		if (initCards(clientfd))
			std::cout << "initCards successfully, clientfd: " << std::endl;
		else
		{
			std::cout << "initCards successfully, clientfd: " << std::endl;
			::close(clientfd);
			return;
		}
			
	}
	
}


bool TCPServer::sendWelcomeMsg(int clientfd)
{
	int sendLenth = ::send(clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
	return sendLenth == static_cast<int>(strlen(PLAYER_WELCOME_MSG));

}

bool TCPServer::initCards(int clientfd)
{
	static constexpr char allCards[] = { 'A','2','3','4','5','6','7','8','9','X','J','Q','K','w','W' };
	static constexpr int allCardsCount = sizeof(allCards) / sizeof(allCards[0]);

	int index1 = rand() % allCardsCount;
	int index2 = rand() % allCardsCount;
	int index3 = rand() % allCardsCount;
	int index4 = rand() % allCardsCount;

	char sendCards[24];
	sprintf(sendCards, "Your cards is: %c %c %c %c\n",
			allCards[index1],
			allCards[index2],
			allCards[index3],
			allCards[index4]);

	int sendLenth = ::send(clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
	return sendLenth == static_cast<int>(strlen(PLAYER_WELCOME_MSG));

}

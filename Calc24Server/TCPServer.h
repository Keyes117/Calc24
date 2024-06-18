#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

class TCPServer
{
public:
	TCPServer() = default;
	~TCPServer() = default;

	TCPServer(const TCPServer& rhs) = delete;
	TCPServer& operator=(const TCPServer& rhs) = delete;

	//移动拷贝构造函数
	TCPServer(TCPServer&& rhs) noexcept = delete;
	//移动operator = 
	TCPServer& operator=(TCPServer&& rhs) noexcept = delete;

	bool init(const std::string& ip, uint16_t port);

	void start();

	//客户端的线程函数
	void clientThreadFunc(int clientfd);

	//发送玩家欢迎消息
	bool sendWelcomeMsg(int clientfd);

	//发牌
	bool initCards(int clientfd);



private:
	int															m_listenfd{ -1 };

	//TODO: 考虑是否可以将std::shared_ptr 改成 unique_ptr
	std::unordered_map<int, std::shared_ptr<std::thread>>		m_clientfdToThread;
};
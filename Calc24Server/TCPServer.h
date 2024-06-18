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

	//�ƶ��������캯��
	TCPServer(TCPServer&& rhs) noexcept = delete;
	//�ƶ�operator = 
	TCPServer& operator=(TCPServer&& rhs) noexcept = delete;

	bool init(const std::string& ip, uint16_t port);

	void start();

	//�ͻ��˵��̺߳���
	void clientThreadFunc(int clientfd);

	//������һ�ӭ��Ϣ
	bool sendWelcomeMsg(int clientfd);

	//����
	bool initCards(int clientfd);



private:
	int															m_listenfd{ -1 };

	//TODO: �����Ƿ���Խ�std::shared_ptr �ĳ� unique_ptr
	std::unordered_map<int, std::shared_ptr<std::thread>>		m_clientfdToThread;
};
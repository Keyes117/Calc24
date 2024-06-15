#pragma once
#include <string>


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
};
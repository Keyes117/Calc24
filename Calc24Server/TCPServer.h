#pragma once
#include <string>


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
};
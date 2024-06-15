#pragma once

#include <string>

class TCPClient
{

public:

	TCPClient() = default;
	~TCPClient() = default;

	TCPClient(const TCPClient& rhs) = delete;
	TCPClient& operator=(const TCPClient& rhs) = delete;

	//�ƶ��������캯��
	TCPClient(TCPClient&& rhs) = delete;
	//�ƶ�operator = 
	TCPClient& operator=(TCPClient&& rhs) = delete;

public:
	bool init(const std::string& serverIP, uint16_t serverPort);
};


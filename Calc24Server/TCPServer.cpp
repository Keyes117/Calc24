#include "TCPServer.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


#include <iostream>
#include <functional>

#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game\n"

bool TCPServer::init(const std::string& ip, uint16_t port)
{

    srand(time(nullptr));

    //1.����һ������socket
    m_listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd == -1)
    {
        std::cout << "create listen socket error." << std::endl;
        return false;
    }
    //�˿ںŸ���
    int optval = 1;
    //TODO: �жϺ����Ƿ���óɹ�
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    ::setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));


    //2.��ʼ����������ַ
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(port);
    if (::bind(m_listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
    {
        std::cout << "bind listen socket error." << std::endl;
        return false;
    }


    //3.��������
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
        //4. ���ܿͻ�������
        int clientfd = ::accept(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if (clientfd == -1)
        {
            std::cout << "accept error" << std::endl;
            return;
        }


            //�����̺߳����󶨲��������⣬
            //���ʹ�þ�̬��������Ӧ�ô���thisָ�룬��Ӧ�ð��̺߳����ڵ��õĺ���Ҳ��ɾ�̬����
            //���ʹ�ó�Ա��������Ӧ���� std::bind ���� ����ָ����в��� thisָ�롣
        auto spThread = std::make_shared<std::thread>(&TCPServer::clientThreadFunc, this, clientfd);
      

        //m_clientfdToThread[clientfd] = spThread;  �������ַ��� Ч���ϲ�����ߵģ� ��Ϊ������һ��thread �Ŀ���
        m_clientfdToThread[clientfd] = std::move(spThread);

}
}

//�ͻ�������ʱ���̺߳���
void TCPServer::clientThreadFunc(int clientfd) {


    std::cout << "new client connected: " << std::endl;

    //�ȳ�ʼ��clientfd��Ӧ��buffer
    m_clientfdToRecvBuf[clientfd] = "";
    //���ͻ�ӭ��Ϣ
    if (!sendWelcomeMsg(clientfd))
    {
        ::close(clientfd);
        return;
    }
    //��ʼ�����Ϳ��� 
    if (initCards(clientfd))
    {
        std::cout << "initCards successfully, clientfd: " << clientfd << std::endl;
    }
    else
    {
        std::cout << "fail to initCards , clientfd: " << clientfd << std::endl;
        ::close(clientfd);
        return;
    }

    while (true) {

        //���ܵ��ͻ�����Ϣ clientMsg
        char clientMsg[32] = { 0 };
        int clientMsgLength = ::recv(clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);

        if (clientMsgLength <= 0) {
            ::close(clientfd);
            return;
        }
        std::string& recvBuf = m_clientfdToRecvBuf[clientfd];
        recvBuf.append(clientMsg, clientMsgLength);

        handleClientMsg(clientfd);
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

    int sendLenth = ::send(clientfd, sendCards, strlen(sendCards), 0);
    return sendLenth == static_cast<int>(strlen(sendCards));

}
// 012 3 456 7  89 	
// ABC\n EDF\n  GH
void TCPServer::handleClientMsg(int clientfd)
{
    std::string currentMsg;
    int index = 0;
    int currentMsgPos = 0;
    std::string& recvBuf = m_clientfdToRecvBuf[clientfd];
    int endOfMsg = recvBuf.length() - 1;

    while (true)
    {
        //���������,�һ�������
        if (recvBuf[index] == '\n')
        {
            currentMsg = recvBuf.substr(currentMsgPos, index - currentMsgPos);

            std::cout << "Client[" << clientfd << "] Says :" << currentMsg << std::endl;

            //ת����ǰ�����Ϣ ���������;
            sendMsgToOtherClients(currentMsg);

            currentMsgPos = index + 1;
        }

        if (index == endOfMsg)
        {
            if (currentMsgPos <= index)
                recvBuf = recvBuf.substr(currentMsgPos, index - currentMsgPos + 1);
            else
                recvBuf.clear();

            break;
        }

        index++;
    }

    return;
}

void TCPServer::sendMsgToOtherClients(const std::string& msg)
{
    for (const auto& client : m_clientfdToThread)
    {
        //TODO: ������Ϣ̫��,һ���Է�����ȥ, ��Ҫ���� 
        int sendLength = ::send(client.first, msg.c_str(), msg.length(), 0);
        if (sendLength != msg.length())
        {
            std::cout << "fails to sendMsgToOtherClients, clientfd:" << client.first << std::endl;
        }
    }
}

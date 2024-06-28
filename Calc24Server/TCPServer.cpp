#pragma once
#include "TCPServer.h"   //当前CPP文件的头文件


#include <arpa/inet.h>     // 系统库文件
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <iostream>         //C++库文件
#include <functional>
#include <sstream>

//其他库文件
#include "Player.h"         //本项目中其他头文件

#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game\n"
#define PLAYER_WAITING_MSG "The desk is not full with 3 player currently, please wait for seconds\n"

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
        int clientfd = ::accept4(m_listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen, SOCK_NONBLOCK);
        if (clientfd == -1)
        {
            std::cout << "accept error" << std::endl;
            return;
        }

        newPlayerJoined(clientfd);

    }
}

//客户端连上时的线程函数
void TCPServer::clientThreadFunc(int clientfd, std::shared_ptr<Desk> spCurrentDesk)
{
    std::cout << "new client connected: " << std::endl;

    //先初始化clientfd对应的buffer
    m_clientfdToRecvBuf[clientfd] = "";

    //初始化clientfd对应的 mutex;
    //m_clientfdToMutex[clientfd] = std::mutex();  mutex不支持拷贝构造
    //m_clientfdToMutex.emplace(clientfd, std::mutex()); //原位构造
    // 改成指针后
    m_clientfdToMutex[clientfd] = std::move(std::make_shared<std::mutex>()); //C++11 的STL容器或自动move指针,可以不显示的move
    //这里线程可能后运行到这里 导致客户线程 无法发牌
    //m_clientfdToDeskReady[clientfd] = false;

    //发送欢迎消息
    if (!sendWelcomeMsg(clientfd))
    {
        ::close(clientfd);
        return;
    }

    bool initCardsCompleted = false;
    bool alreadySentwaitMsg = false;
    //等待满n个人就发牌
    while (true)
    {

        std::atomic<bool>* deskReady;
        {
            std::lock_guard<std::mutex> scopedLock(m_mutexForClientfdToDeskReady);
            deskReady = &m_clientfdToDeskReady[clientfd];
        }

        if (*deskReady && !initCardsCompleted)
        {
            //初始化发送卡牌 
            if (sendCards(clientfd))
            {
                initCardsCompleted = true;
                std::cout << "initCards successfully, clientfd: " << clientfd << std::endl;
                break;
            }
            else
            {
                std::cout << "fail to initCards , clientfd: " << clientfd << std::endl;
                ::close(clientfd);
                return;
            }
        }
        else if (!alreadySentwaitMsg)
        {
            sendWaitMsg(clientfd);
            alreadySentwaitMsg = true;
        }
    }


    while (true) {
        //接受到客户端消息 clientMsg
        char clientMsg[32] = { 0 };
        int clientMsgLength = ::recv(clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);

        if (clientMsgLength == 0) {
            ::close(clientfd);
            return;
        }

        if (clientMsgLength < 0)
        {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                //链接真的出错了
                ::close(clientfd);
                return;
            }
            else
            {
                //sleep不合理，暂时这么用
                sleep(1);
                continue;
            }
        }

        std::string& recvBuf = m_clientfdToRecvBuf[clientfd];
        recvBuf.append(clientMsg, clientMsgLength);

        handleClientMsg(clientfd);
    }
}

void TCPServer::newPlayerJoined(int clientfd)
{
    auto spCurrentPlayer = std::make_shared<Player>(clientfd);
    //m_clientfdToPlayer[clientfd] = std::move(spPlayer);

    std::shared_ptr<Desk> spCurrentFullDesk;

    //如果是第一个桌子
    auto iter = m_deskInfo.rbegin();
    if (iter == m_deskInfo.rend())
    {
        //第一个玩家加入
        auto spNewDesk = std::make_shared<Desk>();
        // 桌子的Id从1开始
        // TODO: 把1改成常量不用
        spNewDesk->id = 1;
        spNewDesk->spPlayer1 = spCurrentPlayer;

        spCurrentPlayer->setDesk(spNewDesk);
        //注意不能 先做move操作
        m_deskInfo.push_back(std::move(spNewDesk));
    }
    //如果是第一个桌子
    else
    {
        //非第一个玩家加入
        if ((*iter)->spPlayer1.expired())  //weak_ptr 管理的对象 已经没有了 通过expired;
        {
            (*iter)->spPlayer1 = spCurrentPlayer;
        }
        else if ((*iter)->spPlayer2.expired())
        {
            (*iter)->spPlayer2 = spCurrentPlayer;
        }
        else if ((*iter)->spPlayer3.expired())
        {
            (*iter)->spPlayer3 = spCurrentPlayer;

            //当前来了新玩家， 桌子坐满了
            spCurrentFullDesk = *iter;
        }
        else
        {
            auto spNewDesk = std::make_shared<Desk>();
            // 桌子的Id从1开始
            // TODO: 把1改成常量不用
            spNewDesk->id = m_deskInfo.size() + 1;
            spNewDesk->spPlayer1 = spCurrentPlayer;

            //注意不能 先做move操作
            m_deskInfo.push_back(std::move(spNewDesk));
        }

    }


    if (spCurrentFullDesk != nullptr)
    {
        std::lock_guard<std::mutex> scopedLock(m_mutexForClientfdToDeskReady);
        initCards(spCurrentFullDesk);
        std::shared_ptr<Player> p1 = spCurrentFullDesk->spPlayer1.lock();
        std::shared_ptr<Player> p2 = spCurrentFullDesk->spPlayer2.lock();
        std::shared_ptr<Player> p3 = spCurrentFullDesk->spPlayer3.lock();
        if (p1 != nullptr && p2 != nullptr && p3 != nullptr)
        {
            p1->serReady(true);
            p2->serReady(true);
            p3->serReady(true);
        }

    }

    //这里线程函数绑定参数的问题，
    //如果使用静态函数，则不应该传入this指针，而应该吧线程函数内调用的函数也变成静态函数
    //如果使用成员函数，这应该用 std::bind 或者 函数指针进行捕获 this指针。
    auto spThread = std::make_shared<std::thread>(&TCPServer::clientThreadFunc, this, clientfd, spCurrentFullDesk);

    //m_clientfdToThread[clientfd] = spThread;  考虑这种方法 效率上不是最高的， 因为会生成一份thread 的拷贝
    m_clientfdToThread[clientfd] = std::move(spThread);


}

bool TCPServer::sendWelcomeMsg(int clientfd)
{
    int sendLength = ::send(clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
    return sendLength == static_cast<int>(strlen(PLAYER_WELCOME_MSG));

}

bool TCPServer::sendWaitMsg(int clientfd)
{

    int sendLength = ::send(clientfd, PLAYER_WAITING_MSG, strlen(PLAYER_WAITING_MSG), 0);
    return sendLength == static_cast<int>(strlen(PLAYER_WAITING_MSG));
}

void TCPServer::initCards(std::shared_ptr<Desk> spDesk)
{
    static constexpr char allCards[] = { 'A','2','3','4','5','6','7','8','9','X','J','Q','K','w','W' };
    static constexpr int allCardsCount = sizeof(allCards) / sizeof(allCards[0]);

    int index1 = rand() % allCardsCount;
    int index2 = rand() % allCardsCount;
    int index3 = rand() % allCardsCount;
    int index4 = rand() % allCardsCount;

    char newCards[24];
    sprintf(newCards, "Your cards is: %c %c %c %c\n",
        allCards[index1],
        allCards[index2],
        allCards[index3],
        allCards[index4]);

    spDesk->m_toSendCards.append(newCards, strlen(newCards));

}

bool TCPServer::sendCards(int clientfd)
{
    int sendLength = ::send(clientfd, m_toSendCards.c_str(), m_toSendCards.length(), 0);
    return sendLength == static_cast<int>(m_toSendCards.length());
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
        //如果是整包,且还有数据
        if (recvBuf[index] == '\n')
        {
            currentMsg = recvBuf.substr(currentMsgPos, index - currentMsgPos);

            std::cout << "Client[" << clientfd << "] Says :" << currentMsg << std::endl;
            currentMsg += "\n";
            //转发当前玩家消息 给其他玩家;
            sendMsgToOtherClients(currentMsg, clientfd);

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

void TCPServer::sendMsgToOtherClients(const std::string& msg, int selfishfd)
{
    //线程安全问题？ 
    //1、多个用户传递消息，可能某个clientfd被多个线程同时调用
    //2、m_clientfdToThread 可能因为客户端丢失连接或者新客户端连接，导致被改写
    int otherClientfd;

    std::lock_guard<std::mutex> scopedLock(m_mutexForClientfdToThread);
    for (const auto& client : m_clientfdToThread)
    {
        otherClientfd = client.first;
        if (otherClientfd == selfishfd)
            continue;

        std::ostringstream oss;
        oss << "Client[" << selfishfd << "] Says :" << msg;

        std::string msgWithOnerInfo = oss.str();

        if (!sendMsgToClient(otherClientfd, msgWithOnerInfo))
            std::cout << "failed to sendMsgToOtherClients, clientfd:" << client.first << "\n";
    }
}

bool TCPServer::sendMsgToClient(int otherClientfd, const std::string& msg)
{
    std::lock_guard<std::mutex> scopedLock(*m_clientfdToMutex[otherClientfd]);

    //TODO: 假设消息太长,一次性发不出去, 需要处理 
    int sendLength = ::send(otherClientfd, msg.c_str(), msg.length(), 0);
    if (sendLength != static_cast<int>(msg.length()))
        return false;

    return true;
}

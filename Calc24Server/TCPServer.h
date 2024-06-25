#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


#define NO_PLAYER_ON_SEAT -1
typedef kClientfd int

struct Desk
{
    int id;
    int clientfd1{ NO_PLAYER_ON_SEAT };
    int clientfd2{ NO_PLAYER_ON_SEAT };
    int clientfd3{ NO_PLAYER_ON_SEAT };
};


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


    /*===============业务方法================*/


    void newPlayerJoined(int clientfd);

    //发送玩家欢迎消息
    bool sendWelcomeMsg(int clientfd);

    bool sendWaitMsg(int clientfd);

    //发牌
    bool initCards(int clientfd);

    //处理客户端信息
    void handleClientMsg(int clientfd);

    //转发消息给其他客户端 
    void sendMsgToOtherClients(const std::string& msg, int selfishfd);

    //向客户端发送消息 
    bool sendMsgToClient(int otherClientfd, const std::string& msg);


private:
    int															m_listenfd{ -1 };

    //TODO: 考虑是否可以将std::shared_ptr 改成 unique_ptr
    std::unordered_map<int, std::shared_ptr<std::thread>>		m_clientfdToThread;

    //保护clientToThread;
    std::mutex                                                  m_mutexForClientfdToThread;

    std::unordered_map<int, std::string>						m_clientfdToRecvBuf;

    std::unordered_map<int, std::shared_ptr<std::mutex>>        m_clientfdToMutex;

    std::vector<Desk>                                           m_deskInfo;


    //key=>clientfd ,value=>clientfd对应的桌子是否可以发牌了
    std::unordered_map<int, std::atomic<bool>>					m_clientfdToDeskReady;

    std::mutex                                                  m_mutexForClientfdToDeskReady;
};
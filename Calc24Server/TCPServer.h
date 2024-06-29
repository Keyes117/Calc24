#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>



typedef int kClientfd;
class Player;
class Desk;

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
    void clientThreadFunc(std::shared_ptr<Player> spCurrentPlayer, std::shared_ptr<Desk> spCurrentDesk);

    /*===============业务方法================*/

    //新玩家加入逻辑
    void newPlayerJoined(int clientfd);

    //初始化卡牌
    void initCards(const std::shared_ptr<Desk>& spDesk);

    //转发消息给其他客户端 
    void sendMsgToOtherClients(const std::string& msg, int selfishfd);


private:
    int															     m_listenfd{ -1 };

    std::unordered_map<kClientfd, std::shared_ptr<Player>>           m_clientfdToPlayer;

    std::mutex                                                       m_mutexForClientfdToPlayer;

    //TODO: 考虑将std::shared_ptr 改成 unique_ptr
    std::unordered_map<kClientfd, std::shared_ptr<std::thread>>		 m_clientfdToThread;

    std::unordered_map<kClientfd, std::string>						 m_clientfdToRecvBuf;

    std::vector<std::shared_ptr<Desk>>                               m_deskInfo;

};
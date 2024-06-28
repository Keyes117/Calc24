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
    void clientThreadFunc(int clientfd, std::shared_ptr<Desk> spCurrentDesk);

    /*===============业务方法================*/

    //新玩家加入逻辑
    void newPlayerJoined(int clientfd);

    //发送玩家欢迎消息
    bool sendWelcomeMsg(int clientfd);

    //发送等待消息
    bool sendWaitMsg(int clientfd);

    //初始化卡牌
    void initCards(std::shared_ptr<Desk> spDesk);

    //发牌
    bool sendCards(int clientfd);

    //处理客户端信息
    void handleClientMsg(int clientfd);

    //转发消息给其他客户端 
    void sendMsgToOtherClients(const std::string& msg, int selfishfd);

    //向客户端发送消息 
    bool sendMsgToClient(int otherClientfd, const std::string& msg);


private:
    int															     m_listenfd{ -1 };

    std::unordered_map<kClientfd, std::shared_ptr<Player>>           m_clientfdToPlayer;


    //TODO: 考虑将std::shared_ptr 改成 unique_ptr
    std::unordered_map<kClientfd, std::shared_ptr<std::thread>>		 m_clientfdToThread;

    //保护clientToThread;
    std::mutex                                                       m_mutexForClientfdToThread;

    std::unordered_map<kClientfd, std::string>						 m_clientfdToRecvBuf;

    std::unordered_map<kClientfd, std::shared_ptr<std::mutex>>       m_clientfdToMutex;

    std::vector<std::shared_ptr<Desk>>                               m_deskInfo;


    //key=>clientfd ,value=>clientfd对应的桌子是否可以发牌了
    std::unordered_map<kClientfd, std::atomic<bool>>				 m_clientfdToDeskReady;

    std::mutex                                                       m_mutexForClientfdToDeskReady;


};
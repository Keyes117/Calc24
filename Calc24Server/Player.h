#pragma once


#include <memory>
#include <string>

class Player;

struct Desk
{
    int                   id;
    //考虑用 Player 还是 clientfd?
    std::weak_ptr<Player> spPlayer1;
    std::weak_ptr<Player> spPlayer2;
    std::weak_ptr<Player> spPlayer3;

    std::string           m_toSendCards;
};

class Player final
{
public:
    Player(int clientfd);
    ~Player();

    //TODO: 拷贝构造和move构造函数

    int getClientfd() const;

    bool getCardSentStatus() const;
    void setCardSentStatus(bool status);

    void setDesk(const std::shared_ptr<Desk>& desk);

    void setReadyStatus(bool ready);
    bool getReadyStatus() const;

    void setWaitSentStatus(bool status);
    bool getWaitSentStatus();

    //发送等待消息
    bool sendWaitMsg();

    //发送玩家欢迎消息
    bool sendWelcomeMsg();

    //发牌
    bool sendCards();

    //接受消息
    bool receiveMessage();

    void handleClientMessage(std::string& aMsg);

    bool sendMsgToClient(const std::string& msg);

private:
    //简单类型 放在前面，  复杂类型 有堆内存的，放在后面(有助于排查内容错误)
    const int                   m_clientfd;
    //玩家所在桌是否人满 可以准备发牌
    bool                        m_readyStatus{ false };
    bool                        m_cardSentStatus{ false };
    bool                        m_waitSentStatus{ false };

    std::string                 m_recvBuffer;
    std::weak_ptr<Desk>         m_spDesk;

};


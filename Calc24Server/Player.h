#pragma once


#include <memory>
#include <string>

#define NO_PLAYER_ON_SEAT -1

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

    void setDesk(const std::shared_ptr<Desk>& desk);


    void serReady(bool ready);

private:
    //简单类型 放在前面，  复杂类型 有堆内存的，放在后面(有助于排查内容错误)
    const int                   m_clientfd;
    bool                        m_readyStatus;

    std::string                 m_recvBuffer;
    std::weak_ptr<Desk>         m_spDesk;

};


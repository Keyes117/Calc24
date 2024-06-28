#pragma once


#include <memory>
#include <string>

#define NO_PLAYER_ON_SEAT -1

struct Desk
{
    int                   id;
    //������ Player ���� clientfd?
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

    //TODO: ���������move���캯��

    void setDesk(const std::shared_ptr<Desk>& desk);


    void serReady(bool ready);

private:
    //������ ����ǰ�棬  �������� �ж��ڴ�ģ����ں���(�������Ų����ݴ���)
    const int                   m_clientfd;
    bool                        m_readyStatus;

    std::string                 m_recvBuffer;
    std::weak_ptr<Desk>         m_spDesk;

};


#pragma once


#include <memory>
#include <string>

class Player;

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

    int getClientfd() const;

    bool getCardSentStatus() const;
    void setCardSentStatus(bool status);

    void setDesk(const std::shared_ptr<Desk>& desk);

    void setReadyStatus(bool ready);
    bool getReadyStatus() const;

    void setWaitSentStatus(bool status);
    bool getWaitSentStatus();

    //���͵ȴ���Ϣ
    bool sendWaitMsg();

    //������һ�ӭ��Ϣ
    bool sendWelcomeMsg();

    //����
    bool sendCards();

    //������Ϣ
    bool receiveMessage();

    void handleClientMessage(std::string& aMsg);

    bool sendMsgToClient(const std::string& msg);

private:
    //������ ����ǰ�棬  �������� �ж��ڴ�ģ����ں���(�������Ų����ݴ���)
    const int                   m_clientfd;
    //����������Ƿ����� ����׼������
    bool                        m_readyStatus{ false };
    bool                        m_cardSentStatus{ false };
    bool                        m_waitSentStatus{ false };

    std::string                 m_recvBuffer;
    std::weak_ptr<Desk>         m_spDesk;

};


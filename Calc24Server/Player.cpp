#include "Player.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>


#define NO_PLAYER_ON_SEAT -1
#define PLAYER_WELCOME_MSG "Welcome to Cal24 Game\n"
#define PLAYER_WAITING_MSG "The desk is not full with 3 player currently, please wait for seconds\n"


Player::Player(int clientfd)
    :m_clientfd(clientfd)
{
}

Player::~Player()
{
    ::close(m_clientfd);
}

//====================  get and set ===================

void Player::setDesk(const std::shared_ptr<Desk>& desk)
{
    m_spDesk = desk;
}



bool Player::getCardSentStatus() const
{
    return m_cardSentStatus;
}

void Player::setCardSentStatus(bool status)
{
    m_cardSentStatus = status;
}

int Player::getClientfd() const
{
    return m_clientfd;
}

void Player::setReadyStatus(bool ready)
{
    m_readyStatus = ready;
}

bool Player::getReadyStatus() const
{
    return m_readyStatus;
}

bool Player::sendWelcomeMsg()
{
    int sendLength = ::send(m_clientfd, PLAYER_WELCOME_MSG, strlen(PLAYER_WELCOME_MSG), 0);
    return sendLength == static_cast<int>(strlen(PLAYER_WELCOME_MSG));

}

void Player::setWaitSentStatus(bool status)
{
    m_waitSentStatus = status;
}

bool Player::getWaitSentStatus()
{
    return m_waitSentStatus;
}

bool Player::sendWaitMsg()
{

    int sendLength = ::send(m_clientfd, PLAYER_WAITING_MSG, strlen(PLAYER_WAITING_MSG), 0);
    if (sendLength == static_cast<int>(strlen(PLAYER_WAITING_MSG)))
    {
        m_waitSentStatus = true;
    }
    else
    {
        m_waitSentStatus = false;
    }

    return m_waitSentStatus;
}

bool Player::sendCards()
{
    std::string toSendCards;
    std::shared_ptr<Desk> spDesk = m_spDesk.lock();

    //玩家不在任何桌子上
    if (spDesk == nullptr)
        return false;

    toSendCards = spDesk->m_toSendCards;

    int cardsStrlen = static_cast<int>(toSendCards.length());
    int sendLength = ::send(m_clientfd, toSendCards.c_str(), cardsStrlen, 0);

    if (sendLength == cardsStrlen)
        m_cardSentStatus = true;
    else
        m_cardSentStatus = false;

    return m_cardSentStatus;
}

bool Player::receiveMessage()
{
    //接受到客户端消息 clientMsg
    char clientMsg[32] = { 0 };
    int clientMsgLength = ::recv(m_clientfd, clientMsg, sizeof(clientMsg) / sizeof(clientMsg[0]), 0);

    if (clientMsgLength == 0)
        return false;

    if (clientMsgLength < 0)
    {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            return false;
        else
        {
            //sleep不合理，暂时这么用
            sleep(1);
            return true;
        }
    }
    m_recvBuffer.append(clientMsg, clientMsgLength);
    return true;

}

void Player::handleClientMessage(std::string& aMsg)
{
    std::string currentMsg;
    int index = 0;
    int currentMsgPos = 0;

    int endOfMsg = m_recvBuffer.length() - 1;
    while (true)
    {
        //如果是整包,且还有数据
        if (m_recvBuffer[index] == '\n')
        {
            currentMsg = m_recvBuffer.substr(currentMsgPos, index - currentMsgPos);

            std::cout << "Client[" << m_clientfd << "] Says :" << currentMsg << std::endl;
            currentMsg += "\n";
            aMsg = currentMsg;
            //拿到完整的包就退出
            currentMsgPos = index + 1;
            break;
            //转发当前玩家消息 给其他玩家;
            //sendMsgToOtherClients(currentMsg, m_clientfd);         
        }
        if (index == endOfMsg)
        {
            if (currentMsgPos <= index)
                m_recvBuffer = m_recvBuffer.substr(currentMsgPos, index - currentMsgPos + 1);
            else
                m_recvBuffer.clear();

            break;
        }

        index++;
    }

    return;
}

bool Player::sendMsgToClient(const std::string& msg)
{
    //std::lock_guard<std::mutex> scopedLock(*m_clientfdToMutex[otherClientfd]);

    //TODO: 假设消息太长,一次性发不出去, 需要处理 
    int sendLength = ::send(m_clientfd, msg.c_str(), msg.length(), 0);

    if (sendLength != static_cast<int>(msg.length()))
        return false;

    return true;
}
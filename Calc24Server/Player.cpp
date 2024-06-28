#include "Player.h"

#include <unistd.h>

Player::Player(int clientfd)
    :m_clientfd(clientfd),
    m_readyStatus(false)
{
}

Player::~Player()
{
    ::close(clientfd);
}

void Player::setDesk(const std::shared_ptr<Desk>& desk)
{
    m_spDesk = desk;
}

void Player::serReady(bool ready)
{
    m_readyStatus = ready;
}

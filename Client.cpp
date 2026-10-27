#include "Client.hpp"

Client::Client() {}

Client::~Client() {}

void Client::setFd(int fd)
{
    this->fd = fd;
}

int Client::getFd() const 
{
    return fd;
}
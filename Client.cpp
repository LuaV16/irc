#include "Client.hpp"

Client::Client() : fd(-1) {}

Client::~Client() {}

void Client::setFd(int fd)
{
    this->fd = fd;
}

int Client::getFd() const 
{
    return fd;
}
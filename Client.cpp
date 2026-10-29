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

void Client::appendInputBuffer(const std::string &input)
{
    inputBuffer += input;
}

const std::string &Client::getInputBuffer() const
{
    return inputBuffer;
}

bool Client::hasCompleteLine() const
{
    return (inputBuffer.find("\r\n") != std::string::npos); // If "\r\n" is found in the inputBuffer, we have a complete line
}

std::string Client::getCompleteLine()
{
    size_t end = inputBuffer.find("\r\n");

    if (end == std::string::npos) // std::string::npos means that the substring was not found
        return "";

    std::string line = inputBuffer.substr(0, end);
    inputBuffer.erase(0, end + 2);

    return line;
}

void Client::appendOutputBuffer(const std::string &output)
{
    outputBuffer += output;
}

const std::string &Client::getOutputBuffer() const
{
    return outputBuffer;
}

void Client::removeOutputBuffer(size_t n)
{
    if (n >= outputBuffer.size())
        outputBuffer.clear();
    else
        outputBuffer.erase(0, n);
}
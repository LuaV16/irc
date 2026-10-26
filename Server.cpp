#include "Server.hpp"

Server::Server(int port, const std::string& password): port(port), password(password)
{
    int serverSocket;
    int reuseAddress = 1;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Creation of server socket (IPv4, TCP, default protocol)
    if (serverSocket == -1)
        throw std::runtime_error("Failed to create socket");

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)); // Set socket option to reuse address
}
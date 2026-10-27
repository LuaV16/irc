#include "Server.hpp"

Server::Server(int port, const std::string& password): port(port), password(password)
{
    int reuseAddress = 1;
    sockaddr_in serverAddress; // Structure that holds the server's address information
    struct pollfd pollFd;

    serverAddress.sin_family = AF_INET; // IPv4
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Accepts connections from any IP address
    serverAddress.sin_port = htons(port); // Convert port number to network byte order
    serverAddress.sin_zero[0] = '\0'; // Zero out the rest of the struct

    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Creation of server socket (IPv4, TCP, default protocol)
    if (serverSocket == -1)
        throw std::runtime_error("Failed to create socket");

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) == -1) // Set socket option to reuse address
        throw std::runtime_error("Failed to set socket option");
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) // Bind the socket to the port and address
        throw std::runtime_error("Failed to bind socket");

    if (listen(serverSocket, SOMAXCONN) == -1) // Sets the server as a passive socket that will listen for incoming connections
        throw std::runtime_error("Failed to listen on socket");

    pollFd.fd = serverSocket; // Set the file descriptor to the server socket
    pollFd.events = POLLIN; // Set the events to listen for incoming connections
    pollFd.revents = 0; // Initialize the returned events to 0
    
}
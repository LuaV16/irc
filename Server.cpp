#include "Server.hpp"
#include "Client.hpp"

volatile sig_atomic_t Server::stopFlag = 0;

void Server::signalHandler(int signum)
{
    (void)signum;
    Server::stopFlag = 1;
}

Server::Server(int port, const std::string& password): port(port), password(password)
{
    int reuseAddress = 1;
    sockaddr_in serverAddress = {}; // Structure that holds the server's address information
    struct pollfd pollFd = {}; // Structure for polling the server socket

    serverAddress.sin_family = AF_INET; // IPv4
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Accepts connections from any IP address
    serverAddress.sin_port = htons(port); // Convert port number to network byte order

    this->serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Creation of server socket (IPv4, TCP, default protocol)
    if (serverSocket == -1)
        throw std::runtime_error("Failed to create socket");

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) == -1) // Set socket option to reuse address
    {
        close(serverSocket);
        throw std::runtime_error("Failed to set socket option");
    }    
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) // Bind the socket to the port and address
    {
        close(serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }    

    if (listen(serverSocket, SOMAXCONN) == -1) // Sets the server as a passive socket that will listen for incoming connections
    {
        close(serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1) // Set the socket to non-blocking mode
    {
        close(serverSocket);
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }

    pollFd.fd = serverSocket; // Set the file descriptor to the server socket
    pollFd.events = POLLIN; // Set the events to listen for incoming connections
    pollFd.revents = 0; // Initialize the returned events to 0

    pollFds.push_back(pollFd); // Add the server socket to the vector of poll file descriptors
}

Server::~Server()
{
    if (serverSocket != -1)
    {
        close(serverSocket); // Close the server socket if it's open
        serverSocket = -1;
    }
}   

void Server::removeClientFd(int clientFd)
{
    for (size_t i = 0; i < clientList.size() ; i++)
    {
        if (clientList[i].getFd() == clientFd)
        {
            clientList.erase(clientList.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < pollFds.size() ; i++)
    {
        if (pollFds[i].fd == clientFd)
        {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }
}

void Server::closeServer()
{
    for (size_t i = 0; i < clientList.size() ; i++)
    {
        close(clientList[i].getFd());
        std::cout << RED << "[ Client " << clientList[i].getFd() << " disconnected ]" << RESET << std::endl;
    }
    if (serverSocket != -1)
    {
        close(serverSocket);
        std::cout << RED << "[ Server socket closed ]" << RESET << std::endl;
    }
}

void Server::acceptNewConnection()
{
    Client client;
    int clientFd;
    sockaddr_in clientAddress = {};
    struct pollfd pollFd;
    socklen_t clientAddressLen = sizeof(clientAddress);

    clientFd = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLen);
    if (clientFd == -1)
        throw std::runtime_error("Failed to accept new connection");

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) // All connections are set to non-blocking
        throw std::runtime_error("Failed to set client socket to non-blocking mode");

    pollFd.fd = clientFd; // Set the file descriptor to the client socket
    pollFd.events = POLLIN; // Set the events to listen for incoming data
    pollFd.revents = 0; // Initialize the returned events to 0

    client.setFd(clientFd);

    pollFds.push_back(pollFd); // Add the client socket to the vector of poll file descriptors
    clientList.push_back(client); // Add the client to the vector of clients

    std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::handleClientData(int clientFd)
{
    char buffer[1024];
    ssize_t bytesRead;
    Client *client = NULL;

    for (size_t i = 0; i < clientList.size(); i++)
    {
        if (clientList[i].getFd() == clientFd)
        {
            client = &clientList[i];
            break;
        }
    }
    if (client == NULL)
    {
        std::cerr << RED << "Client not found for fd: " << clientFd << RESET << std::endl;
        return ;
    }

    bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1)
    {
        std::cerr << "Error reading from client " << clientFd << ": " << strerror(errno) << std::endl;
        return ;
    }
    else if (bytesRead == 0) // Client disconnected
    {
        std::cout << RED << "[ Client " << clientFd << " disconnected ]" << RESET << std::endl;
        close(clientFd);
        removeClientFd(clientFd);
    }
    else // Data received from client
    {
        client->appendInputBuffer(std::string(buffer, bytesRead));
        while (client->hasCompleteLine())
        {
            std::string line = client->getCompleteLine();
            std::cout << "Linea completa: " << line << std::endl;
        }
    }
}

void Server::handleClientOutput(int clientFd)
{
    Client *client = NULL;

    for (size_t i = 0; i < clientList.size(); i++)
    {
        if (clientList[i].getFd() == clientFd)
        {
            client = &clientList[i];
            break;
        }
    }
    if (client == NULL)
    {
        std::cerr << RED << "Client not found for fd: " << clientFd << RESET << std::endl;
        return ;
    }

    if (!client->getOutputBuffer().empty())
    {
        ssize_t bytesSent = send(clientFd, client->getOutputBuffer().c_str(), client->getOutputBuffer().size(), 0);
        if (bytesSent == -1)
        {
            std::cerr << "Error sending to client " << clientFd << ": " << strerror(errno) << std::endl;
            return ;
        }
        client->removeOutputBuffer(static_cast<size_t>(bytesSent)); // Remove the sent data from the output buffer
    }
}

void Server::runServer()
{

    while (stopFlag == 0)
    {
        int pollResult;

        pollResult = poll(&pollFds[0], pollFds.size(), -1);
        if (pollResult == -1)
        {
            if (errno == EINTR) // Poll was interrupted by a signal, it's not a real error
                continue;
            throw std::runtime_error("Poll failed");
        }

        for (int i = 0; i < int(pollFds.size()); i++)
        {
            if (pollFds[i].revents & POLLIN)
            {
                if (pollFds[i].fd == serverSocket) // There's a new incoming connection on the server socket
                    acceptNewConnection(); // Accept the new connection
                else // There's data to read from an existing client socket
                    handleClientData(pollFds[i].fd); // Handle the data from the client
            }
            if (pollFds[i].revents & POLLOUT)
            {
                handleClientOutput(pollFds[i].fd);
            }
        }
    }
    closeServer();
}
#include "Server.hpp"
#include "Client.hpp"

volatile sig_atomic_t Server::stopFlag = 0;

void Server::signalHandler(int signum)
{
    (void)signum;
    Server::stopFlag = 1;
}

Client *Server::getClient(int clientFd)
{
    for (size_t i = 0; i < clientList.size(); i++)
    {
        if (clientList[i].getFd() == clientFd)
            return &clientList[i];
    }
    return NULL;
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
    closeServer();
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
        serverSocket = -1;
    }
    clientList.clear();
    pollFds.clear();
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
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) // No pending connections; this is not an error
            return ;

        if (errno == EINTR) // The accept call was interrupted by a signal; this is not an error
            return ;

        std::cerr << RED << "Error accepting new connection: " << strerror(errno) << RESET << std::endl;
        return ;
    }

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) // All connections are set to non-blocking
    {
        close(clientFd);
        throw std::runtime_error("Failed to set client socket to non-blocking mode");
    }

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
            std::cout << "Received from client " << clientFd << ": " << line << std::endl;
            // CARLOS: Aqui habría que parsear la línea y generar la respuesta
        }
    }
}

void Server::queueClientOutput(int clientFd, const std::string &message) // Function to send a message to a client
{
    for (size_t i = 0; i < clientList.size(); i++)
    {
        if (clientList[i].getFd() == clientFd)
        {
            clientList[i].appendOutputBuffer(message);

            for (size_t j = 0; j < pollFds.size(); j++)
            {
                if (pollFds[j].fd == clientFd)
                {
                    pollFds[j].events |= POLLOUT; // Activate POLLOUT to indicate that there's data to send
                    return ;
                }
            }
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
            close(clientFd);
            removeClientFd(clientFd);
            return ;
        }
        client->removeOutputBuffer(static_cast<size_t>(bytesSent)); // Remove the sent data from the output buffer
        if (client->getOutputBuffer().empty())
        {
            for (size_t i = 0; i < pollFds.size(); i++)
            {
                if (pollFds[i].fd == clientFd)
                {
                    pollFds[i].events &= ~POLLOUT; // Stop listening for POLLOUT events if there's no more data to send
                    break;
                }
            }
        }
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
        for (size_t i = 0; i < pollFds.size();)
        {
            pollfd readyPollFd = pollFds[i];

            if (readyPollFd.revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                if (readyPollFd.fd == serverSocket)
                {
                    std::cerr << RED << "Fatal error on server socket" << RESET << std::endl;
                    stopFlag = 1;
                    break;
                }
                close(readyPollFd.fd);
                removeClientFd(readyPollFd.fd);
            }
            else
            {
                if (readyPollFd.revents & POLLIN)
                {
                    if (readyPollFd.fd == serverSocket)
                        acceptNewConnection();
                    else
                        handleClientData(readyPollFd.fd);
                }

                bool stillPresent = false;

                for (size_t j = 0; j < pollFds.size(); j++)
                {
                    if (pollFds[j].fd == readyPollFd.fd)
                    {
                        stillPresent = true;
                        break;
                    }
                }

                if (stillPresent && (readyPollFd.revents & POLLOUT))
                    handleClientOutput(readyPollFd.fd);
            }

            bool stillPresent = false;

            for (size_t j = 0; j < pollFds.size(); j++)
            {
                if (pollFds[j].fd == readyPollFd.fd)
                {
                    stillPresent = true;
                    break;
                }
            }

            if (stillPresent)
                i++;
        }
    }
    closeServer();
}


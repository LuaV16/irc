#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include <sys/socket.h> // For socket functions and structures
# include <netinet/in.h> // For sockaddr_in struct
# include <string>
# include <cstring>
# include <stdexcept>
# include <poll.h> // For pollfd struct
# include <fcntl.h> // For fcntl function
# include <unistd.h>
# include <vector>
# include <iostream>
# include <csignal>
# include <cerrno>
# include <stdlib.h>


# define GRE "\033[1;32m"
# define RED "\033[0;31m"
# define RESET "\033[0m"

class Server {
    public:
        Server(int port, const std::string& password);
        ~Server();

        void runServer();
        void acceptNewConnection();
        static void signalHandler(int signum);
        void handleClientData(int clientFd);
        void handleClientOutput(int clientFd);
        void removeClientFd(int clientFd);
        void queueClientOutput(int clientFd, const std::string &message);
        void closeServer();
        Client *getClient(int clientFd);
    private:
        int port;
        int serverSocket;
        std::string password;
        std::vector<pollfd> pollFds;
        std::vector<Client> clientList;

        static volatile sig_atomic_t stopFlag; // Flag to indicate when to stop the server
};

#endif
#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h> // For socket functions and structures
# include <netinet/in.h> // For sockaddr_in struct
# include <string>
# include <stdexcept>
# include <poll.h> // For pollfd struct
# include <fcntl.h> // For fcntl function
# include <unistd.h>
# include <vector>

class Server {
    public:
        Server(int port, const std::string& password);
        ~Server();

        void runServer();
        void acceptNewConnection();
    private:
        int port;
        int serverSocket;
        std::string password;
        std::vector<pollfd> pollFds;
};

#endif
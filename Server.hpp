#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <netinet/in.h> // For sockaddr_in struct
# include <string>
# include <stdexcept>
# include <poll.h> // For pollfd struct

class Server {
    public:
        Server(int port, const std::string& password);

        void runServer();
    private:
        int port;
        int serverSocket;
        std::string password;
};

#endif
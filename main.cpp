#include "Server.hpp"
#include "Client.hpp"

int check_port(char *port)
{
    for (size_t i = 0; i < strlen(port); i++)
    {
        if (!isdigit(port[i]))
        {
            std::cout << RED << "Error: Port must be a number" << RESET << std::endl;
            return (1);
        }
    }
    if (atoi(port) < 1024 || atoi(port) > 65535)
    {
        std::cout << RED << "Error: Port must be between 1024 and 65535" << RESET << std::endl;
        return (1);
    }
    return (0);
}

int main(int argc, char *argv[]) 
{
    if (argc != 3)
    {
        std::cout << RED << "Usage: " << argv[0] << " <port> <password>" << RESET << std::endl;
        return (1);
    }
    if (check_port(argv[1]))
        return (1);

    try 
    {
        Server server(atoi(argv[1]), argv[2]);
        signal(SIGINT, Server::signalHandler);
        signal(SIGQUIT, Server::signalHandler);
        signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE so the server doesn't crash when trying to write to a closed socket
        server.runServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error running server: " << e.what() << std::endl;
    }
    
    return (0);
}
#include "Server.hpp"

int main(void) 
{
    Server server(6680, "hola");

    try 
    {
        signal(SIGINT, Server::signalHandler);
        signal(SIGQUIT, Server::signalHandler);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error setting signal handlers: " << e.what() << std::endl;
    }
    server.runServer();

    return (0);
}
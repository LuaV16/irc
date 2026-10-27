#include "Server.hpp"

int main(void) 
{
    Server server(6667, "hola");
    server.runServer();

    return (0);
}
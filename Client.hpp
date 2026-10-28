#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <vector>
# include <poll.h>

class Client {
    public:
        Client();
        ~Client();

        void setFd(int fd);

        int getFd() const;
    private:
        int fd;
};

#endif
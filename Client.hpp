#ifndef CLIENT_HPP
# define CLIENT_HPP

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
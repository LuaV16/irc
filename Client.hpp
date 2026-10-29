#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <vector>
# include <poll.h>
# include <string>

class Client {
    public:
        Client();
        ~Client();

        void setFd(int fd);
        void appendInputBuffer(const std::string &input);
        void appendOutputBuffer(const std::string &output);

        int getFd() const;
        const std::string &getInputBuffer() const;
        const std::string &getOutputBuffer() const;
        void removeOutputBuffer(size_t n);

        bool hasCompleteLine() const;
        std::string getCompleteLine();
    private:
        int fd;
        std::string inputBuffer;
        std::string outputBuffer;
};

#endif
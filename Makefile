NAME = ircserv

SRC = main.cpp Server.cpp Client.cpp src/parser/parser.cpp

OBJ := $(SRC:%.cpp=%.o)

CPPFLAGS = -Iinclude
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re : fclean all
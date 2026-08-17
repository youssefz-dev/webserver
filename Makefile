CPPFLAGS = -Wall -Wextra -Werror -std=c++98
CPP = c++

SRC =  srcs/main.cpp \
       srcs/ConfigParser.cpp \
       srcs/ServerConfig.cpp \
       srcs/LocationConfig.cpp \
	   srcs/Server.cpp \
	   Request.cpp \
	   Response.cpp \
	   cgi.cpp
HEADERS = headers/Configparser.hpp \
		  headers/Serverconfig.hpp \
		  headers/Locationconfig.hpp \
		  headers/Server.hpp \
		  headers/Client.hpp \
		  Request.hpp \
		  Response.hpp 
OBJ = $(SRC:.cpp=.o)

NAME = webserv

all : $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)

%.o:%.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all
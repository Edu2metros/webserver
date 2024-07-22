NAME = webserv
SRC = main.cpp Server.cpp Stream.cpp
OBJ = obj
SRCOBJ = $(SRC:%.cpp=${OBJ}/%.o)
CREATE = mkdir -p $(1)
REMOVE = rm -rf $(1)
FLAG = -Wall -Wextra -Werror -g3
CPP = c++

all: $(NAME)
$(NAME) : $(SRCOBJ)
	$(CPP) $^ -std=c++98 -o $(NAME)
${OBJ}/%.o : %.cpp
	$(call CREATE,${OBJ})
	$(CPP) -std=c++98 $(FLAG) -c $< -o $@
clean:
	$(call REMOVE,${OBJ})
fclean: clean
	$(call REMOVE,${NAME})
re: fclean all

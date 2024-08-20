SRC = main.cpp Channel.cpp Client.cpp Commands.cpp Server.cpp
OBJ_DIR = obj/
OBJ = $(addprefix $(OBJ_DIR), $(SRC:.cpp=.o))
NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
RM = rm -rf
MKDIR_P = mkdir -p

PORT = 6667
PASS = 1


$(OBJ_DIR)%.o: %.cpp
	@$(MKDIR_P) $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled: $<"

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "Compiled: $(NAME)"

all: $(NAME)

clean:
	@$(RM) $(OBJ_DIR)
	@echo "Cleaned object files"

fclean: clean
	@$(RM) $(NAME)

re: fclean all

run: re
	@make clean
	@clear
	@echo "Runing $(NAME):"	
	@echo "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n"
	@./$(NAME) $(PORT) $(PASS)
	@echo "\n________________________________________"

.PHONY: all clean fclean re
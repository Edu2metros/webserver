#------------------------------------------------------------------------------#
#                                  GENERICS                                    #
#------------------------------------------------------------------------------#

.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.PHONY: all clean fclean re debug run gdb

VERBOSE ?= TRUE
ifeq ($(VERBOSE),TRUE)
    HIDE :=
else
    HIDE := @
endif

#------------------------------------------------------------------------------#
#                                VARIABLES                                     #
#------------------------------------------------------------------------------#

CC         := c++
CFLAGS     := -std=c++98 -g3 -Wall -Wextra -Werror
DEBUG_FLAGS := -DDEBUG
RM         := rm -rf
INCLUDE    := -Iinclude

NAME       := webserver
SRCDIR     := src
OBJDIR     := obj
SRCS       := $(wildcard $(SRCDIR)/*.cpp)
OBJS       := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
INC        := $(wildcard include/*.hpp include/*.h)

# Create object directory if it doesn't exist
$(OBJDIR):
	@mkdir -p $(OBJDIR)

#------------------------------------------------------------------------------#
#                                 TARGETS                                      #
#------------------------------------------------------------------------------#

all: $(OBJDIR) $(NAME)

run: all
	@clear
	@./$(NAME) twoServers.conf

$(NAME): $(OBJS)
	$(HIDE)$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@mkdir -p $(@D)
	$(HIDE)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	$(HIDE)$(RM) $(OBJDIR)

fclean: clean
	$(HIDE)$(RM) $(NAME)

re: fclean all

gdb:
	gnome-terminal --maximize -- bash -c "sleep 0.3 && gdb --tui --args ./$(NAME) twoServers.conf"
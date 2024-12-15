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
CFLAGS     := -g3
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

gdb: all
	gdb --tui --args ./$(NAME) Configs/twoServers.conf


# Reset
COLOR_OFF	="\033[0m"       # Text Reset

YELLOW		=		"\033[0;33m"       # Yellow
CYAN		=		"\033[0;36m"         # Cyan

BGREEN		=		"\033[1;32m"       # Green
BWHITE		=		"\033[1;37m"       # White

#---------------------SRCS---------------------#
#	use the NAME of one color in the COLOR section above --> $(COLOR NAME)

DEBUG_COLOR	=		$(CYAN)
LEAKS_COLOR	=		$(YELLOW)

#---------------------SRCS---------------------#

OBJS_DEBUG_DIR	=	objs_debug

SRC_DIR		= 		src

OBJS_DIR	=		${SRC_DIR}/objs

HPPS		=		Channel.hpp        CommandHandler.hpp Server.hpp         User.hpp

SRCS		= 		Channel.cpp        CommandHandler.cpp Server.cpp         User.cpp           main.cpp
			
#---------------------COMPILER---------------------#

# chose a NAME for your program

NAME		=		irc_server

CPP 		= 		@clang++

CFLAGS		=		-Wall -Wextra -Werror

OBJS		=		$(patsubst %.cpp, ${OBJS_DIR}/%.o, ${SRCS})

$(OBJS_DIR)/%.o :	${SRC_DIR}/%.cpp
			@ mkdir -p $(OBJS_DIR)
			$(CPP) $(CFLAGS) -c $< -o $@

OBJS_DEBUG	=		$(patsubst %.cpp, ${OBJS_DEBUG_DIR}/%.o, ${SRCS})

$(OBJS_DEBUG_DIR)/%.o :	${SRC_DIR}/%.cpp
			@ mkdir -p $(OBJS_DEBUG_DIR)
			$(CPP) $(CFLAGS) -g -c $< -o $@

#---------------------COMMANDS---------------------#

all:		$(NAME)

$(NAME):	$(OBJS)
			@ $(CPP) $(CFLAGS) $(OBJS) -o $@
			@ echo $(BGREEN) "$(NAME) compiled successfully ✅" $(COLOR_OFF)

clean:
			@ rm -rf $(OBJS_DIR)
			@ rm -f $(OBJS_DEBUG)
			@ rm -f $(EXTRA_CLEAN)
			@ make clean -C ./bot/
			@ echo $(BWHITE) "$(NAME) .o files ware successfully deleted 📃➡ 🗑" $(COLOR_OFF)

fclean:		clean
			@ rm -rf $(OBJS_DEBUG_DIR)
			@ rm -rf debug.dSYM
			@ rm -f $(NAME)
			@ rm -f insultaBOT
			@ make fclean -C ./bot/
			@ echo $(BWHITE) "$(NAME) file was successfully deleted 🗂 ➡🗑" $(COLOR_OFF)

re:			fclean all
			@ echo $(BGREEN) "$(NAME) recompiled successfully ♻️ ✅" $(COLOR_OFF)

bot:		
			@ make -C ./bot/
			@ mv ./bot/insultaBOT .

debug:		$(OBJS_DEBUG)
			@ $(CPP) $(CFLAGS) $(OBJS_DEBUG) -o $(NAME)
			@ echo $(DEBUG_COLOR) "$(NAME) DEBUG compiled successfully  🚫🐛 ✅"
			@ read -p "Press enter to start debugging..."
			@ lldb ./$(NAME)

.PHONY: 	all clean fclean re norme  debug bot
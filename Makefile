NAME := codexion

CC := cc

src_dir := coders/

src_files := codexion.c

includes_dir = $(src_dir)headers/

ARGS = -Wall -Wextra -Werror -pthread

.PHONY: NAME all clean fclean re debug

$(NAME): all

all:
	$(CC) $(src_files:%=$(src_dir)%) $(ARGS) -I$(includes_dir) -o $(NAME)

debug:
	$(CC) $(src:%=$(src_dir)%) -g $(ARGS) -I$(includes_dir) -o $(NAME)


clean:
	@echo clean object files
	@rm -f $(NAME)
	@echo Succefully removed the object files 


fclean: clean

re: fclean | all
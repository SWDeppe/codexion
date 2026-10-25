NAME := codexion

CC := cc

src_dir := coders/

src_files := codexion.c heapq.c heapq_helper.c parser.c realloc.c error_handeler.c

includes_dir = $(src_dir)headers/

ARGS = -Wall -Wextra -Werror -pthread

RUN ?= 4 4000 500 400 400 4 0 EDF

.PHONY: NAME all clean fclean re debug run

SCREEN_W = $(shell tput cols 2>/dev/null || echo 70)
SCREEN_H = $(shell tput lines 2>/dev/null || echo 40)

$(NAME): all

all:
	$(CC) $(src_files:%=$(src_dir)%) $(ARGS) -I$(includes_dir) -o $(NAME)

debug:
	$(CC) $(src_files:%=$(src_dir)%) -g $(ARGS) -I$(includes_dir) -o $(NAME)


run: ARGS += -DSCREEN_SIZE_W=$(SCREEN_W) -DSCREEN_SIZE_H=$(SCREEN_H)
run: debug
	./$(NAME) $(RUN)

clean:
	@echo clean object files
	@rm -f $(NAME)
	@echo Succefully removed the object files 


fclean: clean

re: fclean | all
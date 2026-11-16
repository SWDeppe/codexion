NAME := codexion

CC := cc

src_dir := coders/
 
src_files := codexion.c codexion_cleanup.c link_base_data.c set_base_values.c alloc_base_data.c setup_threads.c \
	realloc.c parser.c times.c write_error.c \
	error_manager.c error_manag_run.c error_manag_run_help.c error_manag_print.c error_manag_log.c \
	coders.c coder_set_status.c coder_use_dungles.c \
	sheduler.c sheduler_assigne.c sheduler_set_dungles.c sheduler_getter.c sheduler_is_priority.c sheduler_prior_types.c \
	monitor.c monitor_check_stop.c monitor_set_stop.c monitor_start.c

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
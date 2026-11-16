/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 07:49:02 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:36:56 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORS_H
# define ERRORS_H

# include <stdint.h>
# include <pthread.h>
# include <errno.h>

typedef uint8_t	t_error;

# ifndef SCREEN_SIZE_W
#  define SCREEN_SIZE_W 70
# endif

# ifndef SCREEN_SIZE_H
#  define SCREEN_SIZE_H 40
# endif

# define OWNED_ERRORS_CHUNK 4096

# define RED "\033[0;31m"
# define NORMAL "\033[0m"
# define GREEN "\033[0;32m"

enum e_error
{
	SUCCESS = 0,
	MISSING_n_cod = 1,
	MISSING_BOURNOUT_TIME = 2,
	MISSING_COMPILE_TIME = 3,
	MISSING_DEBUG_TIME = 4,
	MISSING_REFACTOR_TIME = 5,
	MISSING_REQUIRED_COMIPLATION = 6,
	MISSING_DUNGLE_TIME = 7,
	MISSING_SHEDULER_TYPE = 8,
	UNKNOWENED_SHEDULER_TYPE = 9,
	HEAPQ_SIZE_TO_LOW,
	CODER_DIED,
	GET_TIME_OF_DAY_ERROR,
	MALLOC_ERROR,
	ERROR_LIST_END,
	PTHREAD_ERROR
};

typedef struct error_manager
{
	char			**owners;
	uint32_t		owner_count;
	t_error			**owned_errors;
	uint32_t		*owned_errors_length;

	char			**logs;
	uint32_t		**logs_args;
	uint32_t		logs_count;
	uint32_t		max_logs;

	pthread_t		thread;
	pthread_mutex_t	manager_mutex;

	pthread_mutex_t	stop_mutex;
	uint8_t			stop;
}	t_errors;

void		run(char *owner, t_error (*func)(void **), void **args);
void		*manager_runtime(void *arg);
int			add_owner(t_errors *errors, char *owner);
void		print(char *msg, uint32_t *args, uint32_t args_count);
t_errors	*init_manager(void);
void		write_error(t_error error);
void		error_table(t_errors *errors);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 07:56:23 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:35:45 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include "errors.h"
# include "parser.h"

# define DUNGLE_UNUSED -1
# define FREE_SPACE -1
// small grace period before the monitor commits to a death verdict:
// gives a sheduler grant that was already in flight (dungles handed
// out, coder not yet woken to record its new compile_done_time) a
// chance to land first. Must stay well under the 10ms detection
// precision budget.
# define DEATH_GRACE_MS 7

enum e_can_take
{
	// managed by coder
	NOT_ASKING,
	USING,
	WAITING,
	STARTUP,
	// managed by sheduler
	CAN_TAKE_DUNGLE,
	ONE_DUNG,
	// managed by monitor
	ENOUTH_COMPILATIONS,
	SIM_STOP,
};

enum e_waiting_time_indexs
{
	COMPILING = 0,
	DEBUGING = 1,
	REFACTORING = 2
};

typedef struct sheduler_s
{
	uint32_t		n_cod;

	// thread
	pthread_t		shedulter_thread;

	// verify who is asking for dungles and give access to dungles
	uint8_t			**can_take_dungles;
	pthread_mutex_t	**can_take_mtx;
	pthread_cond_t	**can_take_cond;
		// optimisation
	pthread_mutex_t	wake_mtx;
	pthread_cond_t	wake_cond;

	// verify the priority of the dungles
	// input
	pthread_mutex_t	**ask_times_mtx;
	uint64_t		**ask_times;

	pthread_mutex_t	**compile_done_times_mtx;
	uint64_t		**compile_done_times;

	uint64_t		dungle_time;

	uint8_t			priority;
	// output
	uint32_t		(*dungles_priority)[2];
	// set a deadlock free dungle assignement
	uint32_t		*dungles;

	// stop simulation
	pthread_mutex_t	*error_stop_mtx;
	uint8_t			*error_code;

	// time relevent structure
	pthread_mutex_t	*start_mtx;
	pthread_cond_t	*start_cond;
	uint64_t		*start;

	pthread_mutex_t	ready_mtx;
	pthread_cond_t	ready_cond;
	uint8_t			ready;

	struct timeval	tv;
}	t_sheduler;

typedef struct assigne_one_s
{
	uint32_t	*could_compile;
	t_sheduler	*sh;
}	t_assign;

typedef struct error_free_s
{
	t_sheduler	*sheduler;
	uint32_t	i;
	uint32_t	dungle;
	uint64_t	now;
	uint64_t	*next;
}	t_error_free;

typedef struct coder_s
{
	int				id;

	// thread
	pthread_t		t_coderhread;

	uint8_t			can_take_dungles;
	pthread_mutex_t	can_take_mtx;
	pthread_cond_t	can_take_cond;

	uint32_t		used_dungle[2];

	pthread_mutex_t	compile_done_time_mtx;
	pthread_mutex_t	compile_done_time_cond;
	uint64_t		compile_done_time;

	uint32_t		demanded_compilations;
	uint32_t		compile_count;

	pthread_mutex_t	*wake_monit_mtx;
	pthread_cond_t	*wake_monit_cond;

	pthread_mutex_t	*wake_shedu_mtx;
	pthread_cond_t	*wake_shedu_cond;

	pthread_mutex_t	ask_time_mtx;
	uint64_t		ask_time;

	uint32_t		dungles[2];

	uint32_t		waiting_times[3];

	pthread_mutex_t	*error_stop_mtx;
	uint8_t			*error_code;

	pthread_mutex_t	*start_mtx;
	pthread_cond_t	*start_cond;

	uint64_t		*start;
	struct timeval	tv;
}	t_coder;

typedef struct monitor_s
{
	uint32_t		n_cod;
	pthread_t		thread;

	uint32_t		burount_time;
	uint64_t		start;

	int64_t			*dies_in;
	uint64_t		min_t;

	pthread_mutex_t	wake_mtx;
	pthread_cond_t	wake_cond;

	pthread_mutex_t	*wake_shedu_mtx;
	pthread_cond_t	*wake_shedu_cond;

	pthread_mutex_t	**compile_done_times_mtx;
	uint64_t		**compile_done_times;

	pthread_mutex_t	error_stop_mtx;
	uint8_t			error_code;

	uint8_t			**can_take_dungles;
	pthread_mutex_t	**can_take_mtx;
	pthread_cond_t	**can_take_cond;

	pthread_mutex_t	start_mtx;
	pthread_cond_t	start_cond;

	pthread_mutex_t	*shedu_ready_mtx;
	pthread_cond_t	*shedu_ready_cond;
	uint8_t			*shedu_ready;

	uint8_t			*coders_done;
	uint32_t		coders_done_count;

	struct timeval	tv;
}	t_monitor;

typedef struct codexion_s
{
	uint32_t		n_cod;
	t_coder			*coders;
	t_sheduler		sheduler;
	t_monitor		monitor;
	t_errors		*err_manager;
}	t_codexion;

typedef struct get_time_s
{
	uint64_t	*ms_time;
}	t_get_tm;

typedef struct s_buf
{
	char		*buf;
	uint32_t	pos;
}	t_buf;

t_error		get_time(void **args);
t_error		ms_to_abstime(void **args);
void		*sheduler_runtime(void *arg);
void		*monitor_runtime(void *arg);
void		*coder_runtime(void *arg);
t_error		set_stat(void **args);
t_error		error_check(void **args);
t_error		wait_for_access(void **args);
int			use_dungles(t_coder *coder, uint64_t ms_time);
t_error		set_dungles(void **args);
uint32_t	left(uint32_t i, uint32_t max);
uint32_t	right(uint32_t i, uint32_t max);
t_error		set_fifo(void **args);
t_error		set_edf(void **args);
t_error		is_stop(void **args);
t_error		is_right_priority(void **args);
t_error		is_left_priority(void **args);
t_error		assigne_one(void **args);
t_error		get_waiters_byte(void **args);
t_error		mark_one_dungle(void **args);
t_error		set_stop(void **args);
uint8_t		check_stop(t_monitor *monitor);
void		set_values(t_codexion *c, struct timeval tv, t_pars parsed);
void		link_data(t_codexion *c);
t_error		alloc_sheduler(t_sheduler *s, uint32_t nc);
t_error		alloc_monitor(t_monitor *m, uint32_t nc);
t_error		setup_threads(t_codexion *c);
t_error		start_monitor(void **args);
void		cleanup(t_codexion *codexion);
t_error		mark_one_dungle(void **args);

#endif
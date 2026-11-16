/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 08:11:00 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:55:59 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include "parser.h"

t_error	is_stop(void **args)
{
	t_sheduler	*sheduler;
	uint8_t		*stop;

	sheduler = (t_sheduler *)args[0];
	stop = (uint8_t *)args[1];
	if (pthread_mutex_lock(sheduler->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	*stop = 0;
	if (*sheduler->error_code != SUCCESS)
		*stop = 1;
	if (pthread_mutex_unlock(sheduler->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	start(void **args)
{
	t_sheduler	*sh;

	sh = (t_sheduler *)args;
	if (pthread_mutex_lock(&sh->ready_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	sh->ready = 1;
	if (pthread_mutex_unlock(&sh->ready_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_wait(sh->start_cond, sh->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	wait_next(void **args)
{
	t_sheduler		*sh;
	uint64_t		next;
	int				err;
	struct timespec	ts;

	sh = (t_sheduler *)args;
	run("sheduler", set_dungles, (void *[]){
		sh, &next});
	if (pthread_mutex_lock(&sh->wake_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (next == sh->dungle_time + 1)
	{
		if (pthread_cond_wait(&sh->wake_cond, &sh->wake_mtx) != SUCCESS)
			return (PTHREAD_ERROR);
	}
	else
	{
		run("sheduler", ms_to_abstime, (void *[]){&ts, &next});
		err = pthread_cond_timedwait(&sh->wake_cond, &sh->wake_mtx, &ts);
		if (err != SUCCESS && err != ETIMEDOUT)
			return (PTHREAD_ERROR);
	}
	if (pthread_mutex_unlock(&sh->wake_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

void	*sheduler_runtime(void *arg)
{
	t_sheduler	*sheduler;
	uint32_t	could_compile;
	uint64_t	next;
	uint8_t		stop;

	sheduler = (t_sheduler *)arg;
	stop = 0;
	run("sheduler", start, (void **)sheduler);
	while (1)
	{
		run("sheduler", is_stop, (void *[]){sheduler, &stop});
		if (stop == 1)
			return (NULL);
		run("sheduler", set_dungles, (void *[]){sheduler, &next});
		could_compile = 1;
		while (could_compile)
			run("sheduler", assigne_one,
				(void **)&(t_assign){&could_compile, sheduler});
		run("sheduler", is_stop, (void *[]){sheduler, &stop});
		if (stop == 1)
			return (NULL);
		run("sheduler", wait_next, (void **)sheduler);
	}
}

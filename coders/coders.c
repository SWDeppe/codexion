/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 03:39:22 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:40:28 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include "errors.h"

static t_error	start(void **args)
{
	t_coder	*coder;

	coder = (t_coder *)args;
	if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	coder->can_take_dungles = NOT_ASKING;
	if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(coder->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_wait(coder->start_cond, coder->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(coder->start_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

t_error	error_check(void **args)
{
	t_coder	*coder;
	uint8_t	*stop;

	coder = (t_coder *)args[0];
	stop = (uint8_t *)args[1];
	if (pthread_mutex_lock(coder->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*coder->error_code != SUCCESS)
		*stop = 1;
	if (pthread_mutex_unlock(coder->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	wait_for_access(void **args)
{
	t_coder	*coder;

	coder = (t_coder *)args;
	if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	while (coder->can_take_dungles != CAN_TAKE_DUNGLE
		&& coder->can_take_dungles != SIM_STOP)
	{
		if (pthread_cond_wait(&coder->can_take_cond, &coder->can_take_mtx)
			!= SUCCESS)
			return (PTHREAD_ERROR);
	}
	coder->used_dungle[0] = coder->dungles[0];
	coder->used_dungle[1] = coder->dungles[1];
	coder->can_take_dungles = USING;
	if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	set_ask_time(void **args)
{
	t_coder		*coder;
	uint64_t	*ms_time;

	coder = (t_coder *)args[0];
	ms_time = (uint64_t *)args[1];
	run("Coder", get_time, (void **)&(t_get_tm){ms_time});
	if (pthread_mutex_lock(&coder->ask_time_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	coder->ask_time = *ms_time;
	if (pthread_mutex_unlock(&coder->ask_time_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

void	*coder_runtime(void *arg)
{
	t_coder		*coder;
	uint64_t	ms_time;
	uint8_t		stop;

	coder = (t_coder *)arg;
	run("Coder", start, (void **)coder);
	stop = 0;
	while (1)
	{
		run("Coder", set_ask_time, (void *[]){coder, &ms_time});
		run("Coder", set_stat, (void *[]){coder, &stop});
		run("Coder", error_check, (void *[]){coder, &stop});
		if (stop)
			return (NULL);
		run("Coder", wait_for_access, (void **)coder);
		run("Coder", error_check, (void *[]){coder, &stop});
		if (stop)
			return (NULL);
		run("Coder", get_time, (void **)&(t_get_tm){&ms_time});
		if (use_dungles(coder, ms_time))
			return (NULL);
	}
}

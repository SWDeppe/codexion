/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_start.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 15:16:08 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 15:16:26 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	count_idle_coders(t_monitor *monitor, uint32_t *count)
{
	uint32_t	i;

	*count = 0;
	i = 0;
	while (i < monitor->n_cod)
	{
		if (pthread_mutex_lock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		if (*monitor->can_take_dungles[i] == NOT_ASKING)
			(*count)++;
		if (pthread_mutex_unlock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	return (SUCCESS);
}

static t_error	count_sheduler_ready(t_monitor *monitor, uint32_t *count)
{
	if (pthread_mutex_lock(monitor->shedu_ready_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*monitor->shedu_ready == 1)
		(*count)++;
	if (pthread_mutex_unlock(monitor->shedu_ready_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	wait_all_ready(t_monitor *monitor)
{
	uint32_t	count;
	t_error		err;

	count = 0;
	while (count < monitor->n_cod + 1)
	{
		err = count_idle_coders(monitor, &count);
		if (err != SUCCESS)
			return (err);
		err = count_sheduler_ready(monitor, &count);
		if (err != SUCCESS)
			return (err);
	}
	return (SUCCESS);
}

static t_error	set_compile_done_times(t_monitor *monitor)
{
	uint32_t	i;

	i = 0;
	while (i < monitor->n_cod)
	{
		if (pthread_mutex_lock(monitor->compile_done_times_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		*monitor->compile_done_times[i] = monitor->start;
		if (pthread_mutex_unlock(monitor->compile_done_times_mtx[i])
			!= SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	return (SUCCESS);
}

t_error	start_monitor(void **args)
{
	t_monitor	*monitor;
	t_error		err;

	monitor = (t_monitor *)args;
	err = wait_all_ready(monitor);
	if (err != SUCCESS)
		return (err);
	run("Monitor", get_time, (void **)&(t_get_tm){&monitor->start});
	err = set_compile_done_times(monitor);
	if (err != SUCCESS)
		return (err);
	if (pthread_cond_broadcast(&monitor->start_cond) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

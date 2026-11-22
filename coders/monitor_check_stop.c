/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_check_stop.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 06:03:11 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 16:58:50 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	get_done_coders(void **args)
{
	t_monitor	*monitor;
	uint32_t	i;

	monitor = (t_monitor *)args[0];
	i = 0;
	while (i < monitor->n_cod)
	{
		if (monitor->coders_done[i] && ++i)
			continue ;
		if (pthread_mutex_lock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		if (*monitor->can_take_dungles[i] == ENOUTH_COMPILATIONS
			&& ++monitor->coders_done_count)
			monitor->coders_done[i] = 1;
		if (pthread_mutex_unlock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (monitor->coders_done_count == monitor->n_cod)
		monitor->error_code = SIM_STOP;
	if (pthread_mutex_unlock(&monitor->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	set_dies_in(void **args)
{
	t_monitor	*monitor;
	uint64_t	*ms_time;
	uint32_t	*i;

	monitor = (t_monitor *)args[0];
	i = (uint32_t *)args[1];
	ms_time = (uint64_t *)args[2];
	run("Monitor", get_time, (void **)&(t_get_tm){ms_time});
	if (pthread_mutex_lock(monitor->compile_done_times_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	monitor->dies_in[*i] = monitor->burount_time + DEATH_GRACE_MS
		- (*ms_time - *monitor->compile_done_times[*i]);
	if (pthread_mutex_unlock(monitor->compile_done_times_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	coder_died(void **args)
{
	t_monitor	*monitor;
	uint64_t	ms_time;
	uint32_t	i;

	monitor = (t_monitor *)args[0];
	i = 0;
	while (i < monitor->n_cod)
	{
		if (monitor->coders_done[i] && ++i)
			continue ;
		run("Monitor", set_dies_in, (void *[]){monitor, &i, &ms_time});
		if (monitor->dies_in[i] < 0)
		{
			if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
				return (PTHREAD_ERROR);
			monitor->error_code = CODER_DIED;
			print("%i Coder %i died !\n",
				(uint32_t []){(uint32_t)ms_time - monitor->start, i}, 2);
			pthread_mutex_unlock(&monitor->error_stop_mtx);
		}
		else if ((uint64_t)monitor->dies_in[i] < monitor->min_t)
			monitor->min_t = monitor->dies_in[i];
		i++;
	}
	return (SUCCESS);
}

uint8_t	check_stop(t_monitor *monitor)
{
	uint8_t	stop;

	monitor->min_t = monitor->burount_time;
	run("Monitor", get_done_coders, (void *[]){monitor});
	run("Monitor", coder_died, (void *[]){monitor});
	stop = 0;
	run("Monitor", set_stop, (void *[]){monitor, &stop});
	return (stop);
}

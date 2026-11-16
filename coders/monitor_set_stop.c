/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_set_stop.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:59:54 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 06:00:04 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	wake_coders(void **args)
{
	uint32_t	i;
	t_monitor	*monitor;

	monitor = (t_monitor *)args[0];
	i = 0;
	while (i < monitor->n_cod)
	{
		if (pthread_mutex_lock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		*monitor->can_take_dungles[i] = (uint8_t)SIM_STOP;
		if (pthread_cond_broadcast(monitor->can_take_cond[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		if (pthread_mutex_unlock(monitor->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	return (SUCCESS);
}

t_error	set_stop(void **args)
{
	t_monitor	*monitor;
	uint8_t		*stop;

	monitor = (t_monitor *)args[0];
	stop = (uint8_t *)args[1];
	if (pthread_mutex_lock(&monitor->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (monitor->error_code != SUCCESS)
	{
		run("Monitor", wake_coders, (void *[]){monitor});
		if (pthread_cond_broadcast(monitor->wake_shedu_cond) != SUCCESS)
			return (PTHREAD_ERROR);
		*stop = 1;
	}
	if (pthread_mutex_unlock(&monitor->error_stop_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

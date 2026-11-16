/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 03:10:03 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 15:16:56 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	wait_next(void **args)
{
	t_monitor		*monitor;
	struct timespec	ts;
	int				err;

	monitor = (t_monitor *)args[0];
	run("Monitor", ms_to_abstime, (void *[]){&ts, &monitor->min_t});
	if (pthread_mutex_lock(&monitor->wake_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	err = pthread_cond_timedwait(&monitor->wake_cond, &monitor->wake_mtx, &ts);
	if (err != SUCCESS && err != ETIMEDOUT)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(&monitor->wake_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

void	*monitor_runtime(void *arg)
{
	t_monitor	*monitor;

	monitor = (t_monitor *)arg;
	run("Monitor", start_monitor, (void **)monitor);
	monitor->coders_done_count = 0;
	while (1)
	{
		if (check_stop(monitor))
			return (NULL);
		run("Monitor", wait_next, (void *[]){monitor});
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   times.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 06:24:18 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 06:28:11 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_error	get_time(void **args)
{
	t_get_tm		*vars;
	struct timeval	tv;

	vars = (t_get_tm *)args;
	if (gettimeofday(&tv, NULL) != 0)
		return (GET_TIME_OF_DAY_ERROR);
	*vars->ms_time = tv.tv_usec / 1000 + tv.tv_sec * 1000;
	return (SUCCESS);
}

t_error	ms_to_abstime(void **args)
{
	struct timespec	*ts;
	uint64_t		*add_ms;
	struct timeval	tv;
	uint64_t		us;

	ts = (struct timespec *)args[0];
	add_ms = (uint64_t *)args[1];
	if (gettimeofday(&tv, NULL) != SUCCESS)
		return (GET_TIME_OF_DAY_ERROR);
	us = (uint64_t)tv.tv_usec + (*add_ms % 1000) * 1000;
	ts->tv_sec = tv.tv_sec + *add_ms / 1000 + us / 1000000;
	ts->tv_nsec = (us % 1000000) * 1000;
	return (SUCCESS);
}

// redefines what "N ms" means for the coders: re-check gettimeofday
// against an absolute target instead of trusting a single usleep() call,
// which only guarantees a minimum, not an exact, duration.
t_error	precise_sleep(uint64_t ms)
{
	uint64_t	start;
	uint64_t	now;
	uint64_t	elapsed;

	if (get_time((void **)&(t_get_tm){&start}) != SUCCESS)
		return (GET_TIME_OF_DAY_ERROR);
	elapsed = 0;
	while (elapsed < ms)
	{
		usleep((ms - elapsed) * 1000);
		if (get_time((void **)&(t_get_tm){&now}) != SUCCESS)
			return (GET_TIME_OF_DAY_ERROR);
		elapsed = now - start;
	}
	return (SUCCESS);
}

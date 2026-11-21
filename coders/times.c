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

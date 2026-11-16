/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_set_dungles.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:47:48 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:47:49 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	error_free(void **args)
{
	t_error_free	*vars;

	vars = (t_error_free *)args;
	if (pthread_mutex_lock(vars->sheduler->compile_done_times_mtx[vars->i])
		!= SUCCESS)
		return (PTHREAD_ERROR);
	if (vars->now - *vars->sheduler->compile_done_times[vars->i]
		>= vars->sheduler->dungle_time)
		vars->sheduler->dungles[vars->dungle] = (uint32_t)DUNGLE_UNUSED;
	else if (*vars->next > vars->sheduler->dungle_time
		- (vars->now - *vars->sheduler->compile_done_times[vars->i]))
		*vars->next = vars->sheduler->dungle_time
			- (vars->now - *vars->sheduler->compile_done_times[vars->i]);
	if (pthread_mutex_unlock(vars->sheduler->compile_done_times_mtx[vars->i])
		!= SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static void	set_dungles_free(uint64_t *next,
	t_sheduler *sheduler, uint32_t i)
{
	uint64_t	now;

	if (sheduler->dungles[left(i, sheduler->n_cod)] == i)
	{
		run("sheduler", get_time, (void **)&(t_get_tm){&now});
		run("sheduler", error_free, (void **)&(t_error_free){sheduler, i,
			left(i, sheduler->n_cod), now, next});
	}
	if (sheduler->dungles[i] == i)
	{
		run("sheduler", get_time, (void **)&(t_get_tm){&now});
		run("sheduler", error_free, (void **)&(t_error_free){sheduler, i,
			i, now, next});
	}
}

t_error	set_dungles(void **args)
{
	uint32_t	i;
	t_sheduler	*sheduler;
	uint64_t	*next;

	sheduler = (t_sheduler *)args[0];
	next = (uint64_t *)args[1];
	i = 0;
	*next = sheduler->dungle_time + 1;
	while (i < sheduler->n_cod)
	{
		if (pthread_mutex_lock(sheduler->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		if (*sheduler->can_take_dungles[i] != USING
			&& *sheduler->can_take_dungles[i] != CAN_TAKE_DUNGLE)
			set_dungles_free(next, sheduler, i);
		if (pthread_mutex_unlock(sheduler->can_take_mtx[i]) != SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	return (SUCCESS);
}

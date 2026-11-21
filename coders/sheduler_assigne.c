/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_assigne.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:46:30 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	broadcast_new_possesor(void **args)
{
	t_sheduler	*sh;
	int			res;
	uint32_t	*could_compile;
	uint32_t	*i;

	sh = (t_sheduler *)args[0];
	could_compile = (uint32_t *)args[1];
	i = (uint32_t *)args[2];
	*could_compile = 1;
	sh->dungles[*i] = *i;
	sh->dungles[left(*i, sh->n_cod)] = *i;
	if (pthread_mutex_lock(sh->can_take_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	*sh->can_take_dungles[*i] = CAN_TAKE_DUNGLE;
	if (pthread_mutex_unlock(sh->can_take_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	res = pthread_cond_broadcast(sh->can_take_cond[*i]);
	if (!(res == SUCCESS))
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static char	is_prioritized_conflict(uint8_t count, uint32_t i, t_sheduler *sh)
{
	uint32_t	res;

	if (!((count >> 1) & 1))
		return (1);
	res = 0;
	run("sheduler", is_left_priority, (void *[]){sh, &count, &i, &res});
	if (res == 1)
		return (1);
	run("sheduler", is_right_priority, (void *[]){sh, &count, &i, &res});
	if (res == 1)
		return (1);
	return (0);
}

static void	try_assign(t_assign *vars, uint32_t i, uint8_t count)
{
	if (vars->sh->dungles[i] != (uint32_t)DUNGLE_UNUSED || vars->sh
		->dungles[left(i, vars->sh->n_cod)] != (uint32_t)DUNGLE_UNUSED)
		return ;
	if (is_prioritized_conflict(count, i, vars->sh))
		return ;
	run("sheduler", broadcast_new_possesor,
		(void *[]){vars->sh, vars->could_compile, &i});
}

static uint8_t	handle_one_cod(t_assign *vars, uint32_t *i)
{
	if (vars->sh->n_cod != 1)
		return (0);
	run("sheduler", mark_one_dungle, (void *[]){vars->sh, i});
	(*i)++;
	return (1);
}

t_error	assigne_one(void **args)
{
	uint32_t	i;
	uint8_t		count;
	uint8_t		prev[2];
	t_assign	*vars;
	uint8_t		stop;

	vars = (t_assign *)args;
	*vars->could_compile = 0;
	run("sheduler", is_stop, (void *[]){vars->sh, &stop});
	if (stop)
		return (CODER_DIED);
	i = 0;
	while (i < vars->sh->n_cod)
	{
		if (handle_one_cod(vars, &i))
			continue ;
		run("sheduler", get_waiters_byte,
			(void *[]){vars->sh, &i, prev, &count});
		try_assign(vars, i, count);
		i++;
	}
	return (SUCCESS);
}

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

// so here wer are using 3 bits
// 1st represent the left coder 2nd represent the coder[i], 3rd the right coder
// left(i) is always right(i - 1) and self(i) is always right-of(i - 1)'s
// right neighbour too, so for i != 0 both bits were already fetched while
// handling i - 1 : only the new "right" bit needs a fresh lock.
// prev[0] = self bit of (i - 1), prev[1] = right bit of (i - 1)
static t_error	get_waiters_byte(void **args)
{
	t_sheduler	*sh;
	uint32_t	*i;
	uint8_t		*prev;
	uint8_t		*count;
	uint8_t		self_bit;

	sh = (t_sheduler *)args[0];
	i = (uint32_t *)args[1];
	prev = (uint8_t *)args[2];
	count = (uint8_t *)args[3];
	if (*i == 0)
	{
		if (pthread_mutex_lock(sh->can_take_mtx[left(*i, sh->n_cod)]) != SUCCESS)
			return (PTHREAD_ERROR);
		*count = (*sh->can_take_dungles[left(*i, sh->n_cod)] == WAITING);
		if (pthread_mutex_unlock(sh->can_take_mtx[left(*i, sh->n_cod)])
			!= SUCCESS)
			return (PTHREAD_ERROR);
		if (pthread_mutex_lock(sh->can_take_mtx[*i]) != SUCCESS)
			return (PTHREAD_ERROR);
		self_bit = (*sh->can_take_dungles[*i] == WAITING);
		if (pthread_mutex_unlock(sh->can_take_mtx[*i]) != SUCCESS)
			return (PTHREAD_ERROR);
	}
	else
	{
		*count = prev[0];
		self_bit = prev[1];
	}
	*count += self_bit * 2;
	if (pthread_mutex_lock(sh->can_take_mtx[right(*i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	prev[1] = (*sh->can_take_dungles[right(*i, sh->n_cod)] == WAITING);
	if (pthread_mutex_unlock(sh->can_take_mtx[right(*i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	*count += prev[1] * 4;
	prev[0] = self_bit;
	return (SUCCESS);
}

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
		if (vars->sh->n_cod == 1)
		{
			run("sheduler", mark_one_dungle, (void *[]){vars->sh, &i});
			i++;
			continue ;
		}
		run("sheduler", get_waiters_byte,
			(void *[]){vars->sh, &i, prev, &count});
		if (vars->sh->dungles[i] == (uint32_t)DUNGLE_UNUSED && vars->sh
			->dungles[left(i, vars->sh->n_cod)] == (uint32_t)DUNGLE_UNUSED)
		{
			if (is_prioritized_conflict(count, i, vars->sh) && ++i)
				continue ;
			run("sheduler", broadcast_new_possesor,
				(void *[]){vars->sh, vars->could_compile, &i});
		}
		i++;
	}
	return (SUCCESS);
}

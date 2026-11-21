/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_waiters_byte.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 00:00:00 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	fetch_left_self(t_sheduler *sh, uint32_t i, uint8_t *count,
	uint8_t *self_bit)
{
	if (pthread_mutex_lock(sh->can_take_mtx[left(i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	*count = (*sh->can_take_dungles[left(i, sh->n_cod)] == WAITING);
	if (pthread_mutex_unlock(sh->can_take_mtx[left(i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->can_take_mtx[i]) != SUCCESS)
		return (PTHREAD_ERROR);
	*self_bit = (*sh->can_take_dungles[i] == WAITING);
	if (pthread_mutex_unlock(sh->can_take_mtx[i]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	fetch_right(t_sheduler *sh, uint32_t i, uint8_t *bit)
{
	if (pthread_mutex_lock(sh->can_take_mtx[right(i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	*bit = (*sh->can_take_dungles[right(i, sh->n_cod)] == WAITING);
	if (pthread_mutex_unlock(sh->can_take_mtx[right(i, sh->n_cod)]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

// so here wer are using 3 bits
// 1st represent the left coder 2nd represent the coder[i], 3rd the right coder
// left(i) is always right(i - 1) and self(i) is always right-of(i - 1)'s
// right neighbour too, so for i != 0 both bits were already fetched while
// handling i - 1 : only the new "right" bit needs a fresh lock.
// prev[0] = self bit of (i - 1), prev[1] = right bit of (i - 1)
t_error	get_waiters_byte(void **args)
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
		if (fetch_left_self(sh, *i, count, &self_bit) != SUCCESS)
			return (PTHREAD_ERROR);
	}
	else
	{
		*count = prev[0];
		self_bit = prev[1];
	}
	if (fetch_right(sh, *i, &prev[1]) != SUCCESS)
		return (PTHREAD_ERROR);
	*count += self_bit * 2 + prev[1] * 4;
	prev[0] = self_bit;
	return (SUCCESS);
}

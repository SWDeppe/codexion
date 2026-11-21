/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_prior_types.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:43:11 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/22 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// always lock the lower coder index first, regardless of which one is
// "first"/"second" semantically: the same pair of coders can be checked
// from either side (is_left_priority vs is_right_priority), and locking
// in a fixed, index-based order avoids a lock-order inversion between
// those two call paths.
static void	sort_pair(uint32_t first, uint32_t second, uint32_t *ord)
{
	if (first < second)
	{
		ord[0] = first;
		ord[1] = second;
	}
	else
	{
		ord[0] = second;
		ord[1] = first;
	}
}

t_error	set_fifo(void **args)
{
	t_sheduler	*sh;
	uint32_t	first;
	uint32_t	second;
	uint32_t	*res;
	uint32_t	ord[2];

	sh = (t_sheduler *)args[0];
	first = (uint32_t)((uint32_t *)args[1])[0];
	second = (uint32_t)((uint32_t *)args[1])[1];
	res = (uint32_t *)args[2];
	if (first == second)
		return (SUCCESS);
	sort_pair(first, second, ord);
	if (pthread_mutex_lock(sh->ask_times_mtx[ord[0]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->ask_times_mtx[ord[1]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*sh->ask_times[first] > *sh->ask_times[second])
		*res = 1;
	if (pthread_mutex_unlock(sh->ask_times_mtx[ord[1]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->ask_times_mtx[ord[0]]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

t_error	set_edf(void **args)
{
	t_sheduler	*sh;
	uint32_t	first;
	uint32_t	second;
	uint32_t	*res;
	uint32_t	ord[2];

	sh = (t_sheduler *)args[0];
	first = (uint32_t)((uint32_t *)args[1])[0];
	second = (uint32_t)((uint32_t *)args[1])[1];
	res = (uint32_t *)args[2];
	if (first == second)
		return (SUCCESS);
	sort_pair(first, second, ord);
	if (pthread_mutex_lock(sh->compile_done_times_mtx[ord[0]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->compile_done_times_mtx[ord[1]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*sh->compile_done_times[first] > *sh->compile_done_times[second])
		*res = 1;
	if (pthread_mutex_unlock(sh->compile_done_times_mtx[ord[1]]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->compile_done_times_mtx[ord[0]]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

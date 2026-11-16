/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_prior_types.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:43:11 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:48:41 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_error	set_fifo(void **args)
{
	t_sheduler	*sh;
	uint32_t	first;
	uint32_t	second;
	uint32_t	*res;

	sh = (t_sheduler *)args[0];
	first = (uint32_t)((uint32_t *)args[1])[0];
	second = (uint32_t)((uint32_t *)args[1])[1];
	res = (uint32_t *)args[2];
	if (pthread_mutex_lock(sh->ask_times_mtx[first]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->ask_times_mtx[second]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*sh->ask_times[first] > *sh->ask_times[second])
		*res = 1;
	if (pthread_mutex_unlock(sh->ask_times_mtx[second]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->ask_times_mtx[first]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

t_error	set_edf(void **args)
{
	t_sheduler	*sh;
	uint32_t	first;
	uint32_t	second;
	uint32_t	*res;

	sh = (t_sheduler *)args[0];
	first = (uint32_t)((uint32_t *)args[1])[0];
	second = (uint32_t)((uint32_t *)args[1])[1];
	res = (uint32_t *)args[2];
	if (pthread_mutex_lock(sh->compile_done_times_mtx[first]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_lock(sh->compile_done_times_mtx[second]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (*sh->compile_done_times[first] > *sh->compile_done_times[second])
		*res = 1;
	if (pthread_mutex_unlock(sh->compile_done_times_mtx[second]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->compile_done_times_mtx[first]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

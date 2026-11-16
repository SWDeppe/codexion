/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_set_status.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:23:54 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:24:28 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	wake_sheduler(void **args)
{
	t_coder	*coder;

	coder = (t_coder *)args;
	if (pthread_mutex_lock(coder->wake_shedu_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_broadcast(coder->wake_shedu_cond) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(coder->wake_shedu_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

t_error	set_stat(void **args)
{
	t_coder	*coder;
	uint8_t	*stop;

	coder = (t_coder *)args[0];
	stop = (uint8_t *)args[1];
	if (coder->compile_count >= coder->demanded_compilations)
	{
		if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
			return (PTHREAD_ERROR);
		coder->can_take_dungles = ENOUTH_COMPILATIONS;
		if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
			return (PTHREAD_ERROR);
		*stop = 1;
	}
	else
	{
		if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
			return (PTHREAD_ERROR);
		coder->can_take_dungles = WAITING;
		if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
			return (PTHREAD_ERROR);
		run("Coder", wake_sheduler, (void **)coder);
	}
	return (SUCCESS);
}

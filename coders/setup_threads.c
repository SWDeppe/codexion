/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_threads.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 14:39:10 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 14:39:12 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	coders_setup_threads(t_codexion *c)
{
	uint32_t	i;

	i = 0;
	while (i < c->n_cod)
	{
		if (pthread_mutex_init(&(c->coders[i].ask_time_mtx), NULL) != SUCCESS)
			return (PTHREAD_ERROR);
		if (pthread_mutex_init(&c->coders[i].can_take_mtx, NULL) != SUCCESS)
			return (PTHREAD_ERROR);
		if (pthread_cond_init(&c->coders[i].can_take_cond, NULL) != SUCCESS)
			return (PTHREAD_ERROR);
		if (pthread_mutex_init(&c->coders[i].compile_done_time_mtx, NULL)
			!= SUCCESS)
			return (PTHREAD_ERROR);
		i++;
	}
	return (SUCCESS);
}

t_error	setup_threads(t_codexion *c)
{
	if (pthread_mutex_init(&c->sheduler.wake_mtx, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_init(&c->sheduler.wake_cond, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_init(&c->monitor.wake_mtx, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_init(&c->monitor.wake_cond, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_init(&c->monitor.start_mtx, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_init(&c->monitor.start_cond, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_init(&c->sheduler.ready_mtx, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_init(&c->sheduler.ready_cond, NULL))
		return (PTHREAD_ERROR);
	if (pthread_mutex_init(&c->monitor.error_stop_mtx, NULL) != SUCCESS)
		return (PTHREAD_ERROR);
	return (coders_setup_threads(c));
}

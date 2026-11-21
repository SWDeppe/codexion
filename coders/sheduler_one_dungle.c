/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_one_dungle.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 00:00:00 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_error	mark_one_dungle(void **args)
{
	t_sheduler	*sh;
	uint32_t	*i;

	sh = (t_sheduler *)args[0];
	i = (uint32_t *)args[1];
	if (pthread_mutex_lock(sh->can_take_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	*sh->can_take_dungles[*i] = ONE_DUNG;
	if (pthread_cond_broadcast(sh->can_take_cond[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(sh->can_take_mtx[*i]) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

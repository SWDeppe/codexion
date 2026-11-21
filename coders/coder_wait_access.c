/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_wait_access.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 00:00:00 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	ack_one_dungle(t_coder *coder, uint8_t *seen)
{
	uint64_t	ms_time;

	if (coder->can_take_dungles != ONE_DUNG || *seen)
		return ;
	*seen = 1;
	run("Coder", get_time, (void **)&(t_get_tm){&ms_time});
	print("%li coder %u took dungle %u !\n",
		(uint32_t []){ms_time - *coder->start, coder->id, coder->id}, 3);
	coder->can_take_dungles = WAITING;
}

t_error	wait_for_access(void **args)
{
	t_coder	*coder;
	uint8_t	one_dung_seen;

	coder = (t_coder *)args;
	one_dung_seen = 0;
	if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	while (coder->can_take_dungles != CAN_TAKE_DUNGLE
		&& coder->can_take_dungles != SIM_STOP)
	{
		ack_one_dungle(coder, &one_dung_seen);
		if (pthread_cond_wait(&coder->can_take_cond, &coder->can_take_mtx)
			!= SUCCESS)
			return (PTHREAD_ERROR);
	}
	coder->used_dungle[0] = coder->dungles[0];
	coder->used_dungle[1] = coder->dungles[1];
	coder->can_take_dungles = USING;
	if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

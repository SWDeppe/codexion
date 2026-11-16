/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sheduler_is_priority.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:51:46 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:56:12 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include "parser.h"

t_error	is_left_priority(void **args)
{
	t_sheduler	*sh;
	uint8_t		*count;
	uint32_t	*i;
	char		*res;

	sh = (t_sheduler *)args[0];
	count = (uint8_t *)args[1];
	i = (uint32_t *)args[2];
	res = (char *)args[3];
	if (*count & 1)
	{
		if (sh->priority == EDF)
			run("sheduler", set_edf, (void *[]){sh, (uint32_t []){
				*i, left(*i, sh->n_cod)
			}, res});
		else
			run("sheduler", set_fifo, (void *[]){sh, (uint32_t []){
				*i, left(*i, sh->n_cod)
			}, res});
	}
	return (SUCCESS);
}

t_error	is_right_priority(void **args)
{
	t_sheduler	*sh;
	uint8_t		*count;
	uint32_t	*i;
	char		*res;

	sh = (t_sheduler *)args[0];
	count = (uint8_t *)args[1];
	i = (uint32_t *)args[2];
	res = (char *)args[3];
	if ((*count >> 2) & 1)
	{
		if (sh->priority == EDF)
			run("sheduler", set_edf, (void *[]){sh, (uint32_t []){
				*i, right(*i, sh->n_cod)
			}, res});
		else
			run("sheduler", set_fifo, (void *[]){sh, (uint32_t []){
				*i, right(*i, sh->n_cod)
			}, res});
	}
	return (SUCCESS);
}

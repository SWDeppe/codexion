/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_base_values.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 14:32:06 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 14:33:57 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include "parser.h"

static void	set_coders_values(t_codexion *c, struct timeval tv, t_pars parsed)
{
	uint32_t	i;

	i = 0;
	while (i < c->n_cod)
	{
		c->coders[i].ask_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		c->coders[i].can_take_dungles = STARTUP;
		c->coders[i].compile_done_time = 0;
		c->coders[i].compile_count = 0;
		c->coders[i].demanded_compilations = parsed.required_compil;
		if (i == 0)
			c->coders[i].dungles[0] = parsed.n_cod - 1;
		else
			c->coders[i].dungles[0] = i - 1;
		c->coders[i].dungles[1] = i;
		c->coders[i].used_dungle[0] = (uint32_t)DUNGLE_UNUSED;
		c->coders[i].used_dungle[1] = (uint32_t)DUNGLE_UNUSED;
		c->coders[i].id = i;
		c->coders[i].waiting_times[COMPILING] = parsed.compile_time;
		c->coders[i].waiting_times[DEBUGING] = parsed.debug_time;
		c->coders[i].waiting_times[REFACTORING] = parsed.refactor_time;
		i++;
	}
}

void	set_values(t_codexion *c, struct timeval tv, t_pars parsed)
{
	uint32_t	i;

	i = 0;
	while (i < c->n_cod)
	{
		c->sheduler.dungles[i] = (uint32_t)DUNGLE_UNUSED;
		c->sheduler.dungles_priority[i][0] = FREE_SPACE;
		c->sheduler.dungles_priority[i][1] = FREE_SPACE;
		c->sheduler.n_cod = parsed.n_cod;
		c->monitor.coders_done[i] = 0;
		i++;
	}
	set_coders_values(c, tv, parsed);
	c->sheduler.dungle_time = parsed.dungle_time;
	c->sheduler.priority = parsed.sheduler;
	c->sheduler.ready = 0;
	c->monitor.min_t = parsed.burnout_time;
	c->monitor.burount_time = parsed.burnout_time;
	c->monitor.n_cod = parsed.n_cod;
	c->monitor.error_code = SUCCESS;
	c->monitor.start = 0;
}

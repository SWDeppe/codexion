/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   link_base_data.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 14:33:35 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 14:35:03 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	link_coder_i(t_codexion *c, uint32_t i)
{
	c->coders[i].wake_shedu_cond = &c->sheduler.wake_cond;
	c->coders[i].wake_shedu_mtx = &c->sheduler.wake_mtx;
	c->coders[i].wake_monit_cond = &c->monitor.wake_cond;
	c->coders[i].wake_monit_mtx = &c->monitor.wake_mtx;
	c->coders[i].start_cond = &c->monitor.start_cond;
	c->coders[i].start_mtx = &c->monitor.start_mtx;
	c->coders[i].start = &c->monitor.start;
	c->coders[i].error_code = &c->monitor.error_code;
	c->coders[i].error_stop_mtx = &c->monitor.error_stop_mtx;
}

static void	link_sheduler_i(t_codexion *c, uint32_t i)
{
	c->sheduler.ask_times[i] = &c->coders[i].ask_time;
	c->sheduler.ask_times_mtx[i] = &c->coders[i].ask_time_mtx;
	c->sheduler.can_take_cond[i] = &c->coders[i].can_take_cond;
	c->sheduler.can_take_dungles[i] = &c->coders[i].can_take_dungles;
	c->sheduler.can_take_mtx[i] = &c->coders[i].can_take_mtx;
	c->sheduler.compile_done_times[i] = &c->coders[i].compile_done_time;
	c->sheduler.compile_done_times_mtx[i] = &c->coders[i].compile_done_time_mtx;
}

static void	link_monitor_i(t_codexion *c, uint32_t i)
{
	c->monitor.can_take_cond[i] = &c->coders[i].can_take_cond;
	c->monitor.can_take_dungles[i] = &c->coders[i].can_take_dungles;
	c->monitor.can_take_mtx[i] = &c->coders[i].can_take_mtx;
	c->monitor.compile_done_times[i] = &c->coders[i].compile_done_time;
	c->monitor.compile_done_times_mtx[i] = &c->coders[i].compile_done_time_mtx;
}

void	link_data(t_codexion *c)
{
	uint32_t	i;

	i = 0;
	while (i < c->n_cod)
	{
		link_coder_i(c, i);
		link_sheduler_i(c, i);
		link_monitor_i(c, i);
		i++;
	}
	c->sheduler.start = &c->monitor.start;
	c->sheduler.start_cond = &c->monitor.start_cond;
	c->sheduler.start_mtx = &c->monitor.start_mtx;
	c->sheduler.error_code = &c->monitor.error_code;
	c->sheduler.error_stop_mtx = &c->monitor.error_stop_mtx;
	c->monitor.shedu_ready = &c->sheduler.ready;
	c->monitor.shedu_ready_mtx = &c->sheduler.ready_mtx;
	c->monitor.shedu_ready_cond = &c->sheduler.ready_cond;
	c->monitor.wake_shedu_cond = &c->sheduler.wake_cond;
	c->monitor.wake_shedu_mtx = &c->sheduler.wake_mtx;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   alloc_base_data.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 14:36:47 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 14:36:52 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_error	alloc_monitor(t_monitor *m, uint32_t nc)
{
	m->can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * nc);
	m->coders_done = (uint8_t *)malloc(sizeof(void *) * nc);
	m->can_take_dungles = (uint8_t **)malloc(sizeof(void *) * nc);
	m->can_take_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * nc);
	m->compile_done_times = (uint64_t **)malloc(sizeof(void *) * nc);
	m->compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * nc);
	m->dies_in = malloc(sizeof(uint64_t) * nc);
	return (SUCCESS);
}

t_error	alloc_sheduler(t_sheduler *s, uint32_t nc)
{
	s->ask_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * nc);
	s->ask_times = (uint64_t **)malloc(sizeof(void *) * nc);
	s->can_take_cond = (pthread_cond_t **)malloc(sizeof(void *) * nc);
	s->can_take_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * nc);
	s->can_take_dungles = (uint8_t **)malloc(sizeof(void *) * nc);
	s->compile_done_times = (uint64_t **)malloc(sizeof(void *) * nc);
	s->compile_done_times_mtx = (pthread_mutex_t **)malloc(sizeof(void *) * nc);
	s->dungles = (uint32_t *)malloc(sizeof(uint32_t) * nc);
	s->dungles_priority = malloc(sizeof(*s->dungles_priority) * nc);
	return (SUCCESS);
}

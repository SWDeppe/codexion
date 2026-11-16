/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion_cleanup.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/16 17:37:04 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:40:31 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_sync(t_codexion *c)
{
	uint32_t	i;

	i = 0;
	while (i < c->n_cod)
	{
		pthread_mutex_destroy(&c->coders[i].ask_time_mtx);
		pthread_mutex_destroy(&c->coders[i].can_take_mtx);
		pthread_cond_destroy(&c->coders[i].can_take_cond);
		pthread_mutex_destroy(&c->coders[i].compile_done_time_mtx);
		i++;
	}
	pthread_mutex_destroy(&c->sheduler.wake_mtx);
	pthread_cond_destroy(&c->sheduler.wake_cond);
	pthread_mutex_destroy(&c->sheduler.ready_mtx);
	pthread_cond_destroy(&c->sheduler.ready_cond);
	pthread_mutex_destroy(&c->monitor.wake_mtx);
	pthread_cond_destroy(&c->monitor.wake_cond);
	pthread_mutex_destroy(&c->monitor.start_mtx);
	pthread_cond_destroy(&c->monitor.start_cond);
	pthread_mutex_destroy(&c->monitor.error_stop_mtx);
}

static void	free_sheduler(t_sheduler *s)
{
	free(s->ask_times_mtx);
	free(s->ask_times);
	free(s->can_take_cond);
	free(s->can_take_mtx);
	free(s->can_take_dungles);
	free(s->compile_done_times);
	free(s->compile_done_times_mtx);
	free(s->dungles);
	free(s->dungles_priority);
}

static void	free_monitor(t_monitor *m)
{
	free(m->can_take_cond);
	free(m->can_take_mtx);
	free(m->can_take_dungles);
	free(m->compile_done_times);
	free(m->compile_done_times_mtx);
	free(m->coders_done);
	free(m->dies_in);
}

static void	free_errors(t_errors *e)
{
	uint32_t	i;

	i = 0;
	while (i < e->owner_count)
	{
		free(e->owners[i]);
		free(e->owned_errors[i]);
		i++;
	}
	i = 0;
	while (i < e->logs_count)
	{
		free(e->logs_args[i]);
		i++;
	}
	free(e->owners);
	free(e->owned_errors);
	free(e->owned_errors_length);
	free(e->logs);
	free(e->logs_args);
	pthread_mutex_destroy(&e->manager_mutex);
	pthread_mutex_destroy(&e->stop_mutex);
	free(e);
}

void	cleanup(t_codexion *codexion)
{
	destroy_sync(codexion);
	free_sheduler(&codexion->sheduler);
	free_monitor(&codexion->monitor);
	free(codexion->coders);
	if (codexion->err_manager)
		free_errors(codexion->err_manager);
}

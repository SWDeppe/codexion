/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 03:36:43 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:36:39 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include "parser.h"
#include "errors.h"

static t_error	setup_data(t_codexion *codexion, t_pars parsed)
{
	t_error			res;
	struct timeval	tv;

	codexion->n_cod = parsed.n_cod;
	codexion->coders = (t_coder *)malloc(sizeof(t_coder) * parsed.n_cod);
	print("__init__", NULL, 0);
	res = alloc_monitor(&codexion->monitor, parsed.n_cod);
	if (res != SUCCESS)
		return (res);
	res = alloc_sheduler(&codexion->sheduler, parsed.n_cod);
	if (res != SUCCESS)
		return (res);
	res = setup_threads(codexion);
	if (res != SUCCESS)
		return (res);
	link_data(codexion);
	if (gettimeofday(&tv, NULL) != 0)
		return (GET_TIME_OF_DAY_ERROR);
	set_values(codexion, tv, parsed);
	return (SUCCESS);
}

static t_error	thread_create(t_codexion *codexion)
{
	uint32_t	i;

	pthread_create(&codexion->sheduler.shedulter_thread, NULL,
		sheduler_runtime, &codexion->sheduler);
	pthread_create(&codexion->monitor.thread, NULL,
		monitor_runtime, &codexion->monitor);
	i = 0;
	while (i < codexion->n_cod)
	{
		pthread_create(&codexion->coders[i].t_coderhread, NULL,
			coder_runtime, &codexion->coders[i]);
		i++;
	}
	return (SUCCESS);
}

static t_error	launch(t_codexion *codexion)
{
	uint32_t	i;
	t_errors	*errm;

	thread_create(codexion);
	pthread_join(codexion->monitor.thread, NULL);
	pthread_join(codexion->sheduler.shedulter_thread, NULL);
	i = 0;
	while (i < codexion->n_cod)
	{
		pthread_join(codexion->coders[i].t_coderhread, NULL);
		i++;
	}
	errm = init_manager();
	if (!errm)
		return (MALLOC_ERROR);
	codexion->err_manager = errm;
	if (pthread_mutex_lock(&errm->stop_mutex) != SUCCESS)
		return (PTHREAD_ERROR);
	errm->stop = 1;
	if (pthread_mutex_unlock(&errm->stop_mutex) != SUCCESS)
		return (PTHREAD_ERROR);
	pthread_join(errm->thread, NULL);
	return (SUCCESS);
}

int	main(int argc, char **argv)
{
	t_pars		parsing;
	t_codexion	proj_holder;
	t_error		res;

	res = parse(argc, argv, &parsing);
	if (res != SUCCESS)
		return (send_error(res), -1);
	setup_data(&proj_holder, parsing);
	launch(&proj_holder);
	cleanup(&proj_holder);
}

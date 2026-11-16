/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manager.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 03:24:43 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:38:55 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errors.h"
#include "realloc.h"
#include "codexion.h"

void	*manager_runtime(void *arg)
{
	t_errors	*errors;

	errors = (t_errors *)arg;
	if (!errors)
		return (NULL);
	while (1)
	{
		pthread_mutex_lock(&errors->manager_mutex);
		if (errors->logs_count > 0 || errors->owner_count > 0)
			error_table(errors);
		pthread_mutex_unlock(&errors->manager_mutex);
		usleep(7000);
		pthread_mutex_lock(&errors->stop_mutex);
		if (errors->stop)
		{
			error_table(errors);
			pthread_mutex_unlock(&errors->stop_mutex);
			return (NULL);
		}
		pthread_mutex_unlock(&errors->stop_mutex);
	}
}

static uint8_t	init_stop(t_errors *manager)
{
	if (pthread_mutex_init(&manager->stop_mutex, NULL) != SUCCESS)
		return (1);
	manager->stop = 0;
	return (0);
}

t_errors	*init_manager(void)
{
	static t_errors	*manager;

	if (manager != NULL)
		return (manager);
	manager = (t_errors *)malloc(sizeof(t_errors));
	if (!manager
		|| pthread_mutex_init(&manager->manager_mutex, NULL) != SUCCESS)
		return (NULL);
	if (init_stop(manager))
		return (NULL);
	manager->owners = NULL;
	manager->owner_count = 0;
	manager->owned_errors = NULL;
	manager->owned_errors_length = NULL;
	manager->logs_count = 0;
	manager->max_logs = 2;
	manager->logs = (char **)malloc(sizeof(char *) * manager->max_logs);
	manager->logs_args = (uint32_t **)malloc(sizeof(void *)
			* manager->max_logs);
	if (!manager->logs || !manager->logs_args)
		return (NULL);
	if (pthread_create(&manager->thread, NULL, manager_runtime, manager)
		!= SUCCESS)
		return (NULL);
	return (manager);
}

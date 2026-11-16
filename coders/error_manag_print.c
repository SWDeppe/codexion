/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manag_print.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 15:21:20 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:22:14 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errors.h"
#include "realloc.h"
#include "codexion.h"

// prints to stdint !
// message "__init__", defines the manager ptr and launches a new thread !
static void	grow_logs(t_errors *manager)
{
	uint32_t	new_max;

	if (manager->logs_count < manager->max_logs)
		return ;
	new_max = manager->max_logs * 2;
	if (new_max == 0)
		new_max = 2;
	manager->logs = (char **)my_realloc(manager->logs,
			manager->logs_count * sizeof(char *),
			new_max * sizeof(char *));
	manager->logs_args = (uint32_t **)my_realloc(manager->logs_args,
			manager->logs_count * sizeof(void *),
			new_max * sizeof(void *));
	manager->max_logs = new_max;
}

static void	store_log(t_errors *manager, char *msg, uint32_t *args,
		uint32_t args_count)
{
	uint32_t	*args_cpy;

	if (!manager->logs || !manager->logs_args)
		return ;
	manager->logs[manager->logs_count] = msg;
	args_cpy = (uint32_t *)malloc(sizeof(uint32_t) * args_count);
	while (args_count--)
		args_cpy[args_count] = args[args_count];
	manager->logs_args[manager->logs_count] = args_cpy;
	manager->logs_count++;
}

void	print(char *msg, uint32_t *args, uint32_t args_count)
{
	static t_errors	*manager;

	if (!msg)
		return ;
	if (strcmp(msg, "__init__") == 0)
	{
		manager = init_manager();
		run("__init__", NULL, (void **)manager);
		return ;
	}
	if (manager == NULL)
	{
		write(2, "Error manager used not initialized\n", 35);
		manager = init_manager();
		run("__init__", NULL, (void **)manager);
		if (!manager)
			return ;
	}
	pthread_mutex_lock(&manager->manager_mutex);
	grow_logs(manager);
	store_log(manager, msg, args, args_count);
	pthread_mutex_unlock(&manager->manager_mutex);
}

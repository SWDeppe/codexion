/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manag_run.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 15:21:26 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 15:36:28 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errors.h"
#include "realloc.h"
#include "codexion.h"

static uint32_t	find_owner(t_errors *errors, char *owner)
{
	uint32_t	i;

	i = 0;
	while (i < errors->owner_count)
	{
		if (strcmp(errors->owners[i], owner) == 0)
			break ;
		i++;
	}
	return (i);
}

static void	store_error(t_errors *errors, uint32_t i, t_error err)
{
	if (errors->owned_errors_length[i] % OWNED_ERRORS_CHUNK == 0)
		errors->owned_errors[i] = (t_error *)my_realloc(errors->owned_errors[i],
				errors->owned_errors_length[i] * sizeof(t_error),
				(errors->owned_errors_length[i]
					+ OWNED_ERRORS_CHUNK) * sizeof(t_error));
	if (errors->owned_errors[i])
	{
		errors->owned_errors[i][errors->owned_errors_length[i]] = err;
		errors->owned_errors_length[i]++;
	}
}

static void	record_error(t_errors *errors, char *owner, t_error err)
{
	uint32_t	i;

	pthread_mutex_lock(&errors->manager_mutex);
	i = find_owner(errors, owner);
	if (i == errors->owner_count && !add_owner(errors, owner))
	{
		pthread_mutex_unlock(&errors->manager_mutex);
		return ;
	}
	store_error(errors, i, err);
	pthread_mutex_unlock(&errors->manager_mutex);
}

void	run(char *owner, t_error (*func)(void **), void **args)
{
	static t_errors	*errors;
	t_error			err;

	if (!owner)
		return ;
	if (strcmp("__init__", owner) == 0)
	{
		errors = init_manager();
		return ;
	}
	if (!errors)
	{
		write(2, "Error manager used not initialized\n", 35);
		errors = init_manager();
		if (!errors)
			return ;
	}
	if (!func)
		return ;
	err = func(args);
	record_error(errors, owner, err);
}

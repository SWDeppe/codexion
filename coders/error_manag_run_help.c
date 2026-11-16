/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manag_run_help.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 15:29:26 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 15:36:04 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "realloc.h"
#include "codexion.h"

static int	grow_owner_arrays(t_errors *errors)
{
	errors->owners = (char **)my_realloc(errors->owners,
			errors->owner_count * sizeof(char *),
			(errors->owner_count + 1) * sizeof(char *));
	errors->owned_errors = (t_error **)my_realloc(errors->owned_errors,
			errors->owner_count * sizeof(t_error *),
			(errors->owner_count + 1) * sizeof(t_error *));
	errors->owned_errors_length = (uint32_t *)my_realloc(
			errors->owned_errors_length,
			errors->owner_count * sizeof(uint32_t),
			(errors->owner_count + 1) * sizeof(uint32_t));
	return (errors->owners && errors->owned_errors
		&& errors->owned_errors_length);
}

static void	set_owner_name(t_errors *errors, uint32_t i, char *owner)
{
	uint32_t	j;
	uint32_t	len_owner;

	len_owner = strlen(owner);
	errors->owners[i] = (char *)malloc(len_owner + 1);
	if (errors->owners[i])
	{
		j = 0;
		while (j < len_owner)
		{
			errors->owners[i][j] = owner[j];
			j++;
		}
		errors->owners[i][j] = '\0';
	}
}

int	add_owner(t_errors *errors, char *owner)
{
	uint32_t	i;

	if (!grow_owner_arrays(errors))
		return (0);
	i = errors->owner_count;
	set_owner_name(errors, i, owner);
	errors->owned_errors[i] = NULL;
	errors->owned_errors_length[i] = 0;
	errors->owner_count++;
	return (1);
}

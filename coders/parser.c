/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 03:09:22 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 15:58:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"
#include "errors.h"

void	send_error(t_error error)
{
	if (error == MISSING_n_cod)
		printf("%sMising nb coders parameter\n%s", RED, NORMAL);
	else if (error == MISSING_BOURNOUT_TIME)
		printf("%sMising burnout time parameter\n%s", RED, NORMAL);
	else if (error == MISSING_COMPILE_TIME)
		printf("%sMising compile time parameter\n%s", RED, NORMAL);
	else if (error == MISSING_DEBUG_TIME)
		printf("%sMising debug time parameter\n%s", RED, NORMAL);
	else if (error == MISSING_REFACTOR_TIME)
		printf("%sMising refactore time parameter\n%s", RED, NORMAL);
	else if (error == MISSING_REQUIRED_COMIPLATION)
		printf("%sMising required compilation parameter\n%s", RED, NORMAL);
	else if (error == MISSING_DUNGLE_TIME)
		printf("%sMising dungle time parameter\n%s", RED, NORMAL);
	else if (error == MISSING_SHEDULER_TYPE)
		printf("%sMising sheduler type parameter\n%s", RED, NORMAL);
	else if (error == UNKNOWENED_SHEDULER_TYPE)
		printf("%sUnknowened sheduler type parameter\n%s", RED, NORMAL);
	else if (error == INVALID_NUMBER)
		printf("%sNumber is invalid\n%s", RED, NORMAL);
}

t_error	get_unsigned(unsigned int *holder, char *from)
{
	int	res;

	res = atoi(from);
	if (res < 0)
		return (INVALID_NUMBER);
	if (res == INT32_MAX && !strcmp(from, "2147483647"))
		return (INVALID_NUMBER);
	*holder = (unsigned int)res;
	return (SUCCESS);
}

t_error	parse(int argc, char **argv, t_pars *pars_values)
{
	if (argc < 9)
		return (argc - 1);
	if (get_unsigned(&pars_values->n_cod, argv[1]) != SUCCESS
		|| pars_values->n_cod == 0)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->burnout_time, argv[2]) != SUCCESS)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->compile_time, argv[3]) != SUCCESS)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->debug_time, argv[4]) != SUCCESS)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->refactor_time, argv[5]) != SUCCESS)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->required_compil, argv[6]) != SUCCESS)
		return (INVALID_NUMBER);
	if (get_unsigned(&pars_values->dungle_time, argv[7]) != SUCCESS)
		return (INVALID_NUMBER);
	if (strcmp(argv[8], "FIFO") == 0)
		pars_values->sheduler = FIFO;
	else if (strcmp(argv[8], "EDF") == 0)
		pars_values->sheduler = EDF;
	else
		return (UNKNOWENED_SHEDULER_TYPE);
	return (SUCCESS);
}

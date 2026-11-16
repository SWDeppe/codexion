/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manag_log.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/16 16:06:34 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/16 17:39:45 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errors.h"
#include "codexion.h"

static uint32_t	print_log(uint32_t space, t_errors *errors, uint32_t log)
{
	uint32_t	i;
	uint32_t	y;
	uint32_t	len;

	i = 0;
	y = 0;
	if (log >= errors->logs_count)
		return (0);
	len = 0;
	while (errors->logs[log][y] && space > len)
	{
		if (errors->logs[log][y] == '%')
		{
			len += printf("%u", errors->logs_args[log][i++]);
			while (errors->logs[log][y] != ' ' && errors->logs[log][y])
				y++;
		}
		if (errors->logs[log][y] == '\n' && y++)
			continue ;
		printf("%c", errors->logs[log][y++]);
		len++;
	}
	return (len);
}

static void	print_err_table_line(uint32_t *colon,
								uint32_t *line,
								t_errors *errors)
{
	printf("|");
	(*colon)++;
	if ((*line % (SCREEN_SIZE_H / errors->owner_count) == 1 || *line == 0)
		&& *line / (SCREEN_SIZE_H / errors->owner_count - 1)
		< errors->owner_count)
		printf("%s",
			errors->owners[*line / (SCREEN_SIZE_H / errors->owner_count - 1)]);
	else if (errors->owned_errors && errors->owned_errors_length
		&& errors->owned_errors[*line / (SCREEN_SIZE_H / errors->owner_count)]
		&& *line % (SCREEN_SIZE_H / errors->owner_count) > 1
		&& errors->owned_errors_length[*line / (SCREEN_SIZE_H
				/ errors->owner_count)]
		>= *line % (SCREEN_SIZE_H / errors->owner_count) - 1)
	{
		printf("call nb %u: ",
			errors->owned_errors_length[*line / (SCREEN_SIZE_H
				/ errors->owner_count)]
			- (*line % (SCREEN_SIZE_H / errors->owner_count) - 1));
		write_error(
			errors->owned_errors[*line / (SCREEN_SIZE_H / errors->owner_count)]
		[errors->owned_errors_length[*line / (SCREEN_SIZE_H
				/ errors->owner_count)] - (*line % (SCREEN_SIZE_H
					/ errors->owner_count) - 1)]
			);
	}
}

void	error_table(t_errors *errors)
{
	uint32_t	line;
	uint32_t	colon;

	if (errors->owner_count == 0)
		return ;
	line = 0;
	colon = 0;
	printf("\033[2J\033[H");
	while (line < SCREEN_SIZE_H)
	{
		colon = print_log(40, errors, (errors->logs_count > SCREEN_SIZE_H - 1)
				* (errors->logs_count - SCREEN_SIZE_H) + line);
		while (colon++ < 40)
			printf(" ");
		if (line % (SCREEN_SIZE_H / errors->owner_count) == 0 && line != 0)
		{
			printf("+");
			while (colon++ < SCREEN_SIZE_W)
				printf("-");
		}
		else
			print_err_table_line(&colon, &line, errors);
		printf("\n");
		line++;
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_manag_print_release.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 00:00:00 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/21 00:00:00 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errors.h"
#include "codexion.h"

static uint32_t	write_uint(char *buf, uint32_t pos, uint32_t value)
{
	char		tmp[10];
	uint32_t	len;

	len = 0;
	if (value == 0)
		tmp[len++] = '0';
	while (value > 0)
	{
		tmp[len++] = '0' + (value % 10);
		value /= 10;
	}
	while (len > 0)
		buf[pos++] = tmp[--len];
	return (pos);
}

static uint32_t	write_arg(char *buf, uint32_t pos, char **msg,
	uint32_t *args, uint32_t *i)
{
	pos = write_uint(buf, pos, args[(*i)++]);
	while (**msg && **msg != ' ')
		(*msg)++;
	return (pos);
}

void	print(char *msg, uint32_t *args, uint32_t args_count)
{
	char		buf[512];
	uint32_t	pos;
	uint32_t	i;

	(void)args_count;
	if (!msg || strcmp(msg, "__init__") == 0)
		return ;
	pos = 0;
	i = 0;
	while (*msg && pos < sizeof(buf) - 1)
	{
		if (*msg == '%')
			pos = write_arg(buf, pos, &msg, args, &i);
		else
			buf[pos++] = *msg++;
	}
	buf[pos] = '\0';
	printf("%s", buf);
}

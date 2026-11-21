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

static void	write_uint(t_buf *b, uint32_t value)
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
		b->buf[b->pos++] = tmp[--len];
}

static void	write_arg(t_buf *b, char **msg, uint32_t *args, uint32_t *i)
{
	write_uint(b, args[(*i)++]);
	while (**msg && **msg != ' ')
		(*msg)++;
}

void	print(char *msg, uint32_t *args, uint32_t args_count)
{
	char		buf[512];
	t_buf		b;
	uint32_t	i;

	(void)args_count;
	if (!msg || strcmp(msg, "__init__") == 0)
		return ;
	b.buf = buf;
	b.pos = 0;
	i = 0;
	while (*msg && b.pos < sizeof(buf) - 1)
	{
		if (*msg == '%')
			write_arg(&b, &msg, args, &i);
		else
			buf[b.pos++] = *msg++;
	}
	buf[b.pos] = '\0';
	printf("%s", buf);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/28 19:37:39 by sdeppe            #+#    #+#             */
/*   Updated: 2026/05/28 19:38:34 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "imports.h"

void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t				i;
	unsigned char		*p_dest;
	unsigned const char	*p_src;

	i = 0;
	p_dest = dest;
	p_src = src;
	if (dest == NULL
		&& src == NULL)
		return (NULL);
	while (i < n)
	{
		p_dest[i] = p_src[i];
		i++;
	}
	return (dest);
}

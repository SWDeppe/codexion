/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_garbage_colector.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 15:02:42 by swann             #+#    #+#             */
/*   Updated: 2026/05/28 20:30:37 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "structs.h"
#include "functions.h"

// !! --> All pointers will be NULL terminated IF OVERRITEN  ->  SEGFAULT <-- !!
// !!   All pointers used as which have to been initilised by this function   !!
// !!   					RETRUN NULL if error						      !!
// which is NULL for a new one 
// else set the pointer you need to reallloc(keeps the initial value)
// if malloc fails the programme will end
// set siz to FREE to remove all the previous data also exists the programme
static void	*proper_terminate(t_ptr **pointers, size_t ptr_count)
{
	while (ptr_count-- > 0)
		free((*pointers)[ptr_count].ptr);
	free(*pointers);
	return NULL;
	// exit(0);
}

void	*ft_realloc(size_t siz, size_t prev_siz, void *mem)
{
	char	*tmp;

	tmp = (char *)malloc(siz);
	if (tmp == NULL)
		return (NULL);
	ft_memcpy(tmp, mem, prev_siz);
	free(mem);
	return (tmp);
}

static void	*generate_new(t_ptr **pointers, size_t *ptr_count, size_t siz)
{
	(*ptr_count)++;
	*pointers = ft_realloc(*ptr_count * sizeof(t_ptr),
			(*ptr_count - 1) * sizeof(t_ptr), *pointers);
	(*pointers)[*ptr_count - 1].siz = siz;
	(*pointers)[*ptr_count - 1].ptr = (void *)malloc(siz);
	if ((*pointers)[*ptr_count - 1].ptr == NULL)
		proper_terminate(pointers, *ptr_count);
	return ((*pointers)[*ptr_count - 1].ptr);
}

void	*gc_malloc(size_t siz, void	*which)
{
	static t_ptr	*pointers;
	static size_t	ptr_count;
	void			*res;
	size_t			i;

	if (siz == (size_t)FREE)
		return (proper_terminate(&pointers, ptr_count));
	if (!which)
		return (generate_new(&pointers, &ptr_count, siz));
	else if (pointers)
	{
		i = 0;
		while (i < ptr_count && which != pointers[i].ptr)
			i++;
		if (i == ptr_count)
			return (NULL);
		res = ft_realloc(siz, pointers[i].siz, pointers[i].ptr);
		pointers[i].ptr = res;
		pointers[i].siz = siz;
		if (!res)
			return (proper_terminate(&pointers, ptr_count));
		return (res);
	}
	return (NULL);
}

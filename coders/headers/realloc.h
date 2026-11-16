/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 07:35:16 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 02:43:56 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REALLOC_H
# define REALLOC_H

# include "stdint.h"
# include "stdlib.h"

void	*my_realloc(void *ptr, uint32_t old_size, uint32_t new_size);

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_error.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 15:40:32 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 15:41:28 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	write_error(t_error error)
{
	if (error == SUCCESS)
		printf("%sSUCCESS%s", GREEN, NORMAL);
	else if (error >= MISSING_n_cod && error <= UNKNOWENED_SHEDULER_TYPE)
		printf("%sParsing error...%s", RED, NORMAL);
	else if (error == HEAPQ_SIZE_TO_LOW)
		printf("%sHEAPQ error (max siz to low)%s", RED, NORMAL);
	else if (error == CODER_DIED)
		printf("%sStop error (a coder died)%s", RED, NORMAL);
	else if (error == GET_TIME_OF_DAY_ERROR)
		printf("%sgettimeofday() function failed%s", RED, NORMAL);
	else if (error == MALLOC_ERROR)
		printf("%smemory allocation's function failed%s", RED, NORMAL);
	else if (error == PTHREAD_ERROR)
		printf("%spthread lib failed%s", RED, NORMAL);
}

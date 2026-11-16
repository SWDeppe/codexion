/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/12 07:38:43 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 06:21:25 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include "errors.h"
# include "string.h"
# include "stdlib.h"
# include "unistd.h"
# include "stdio.h"

typedef enum shedule_e
{
	FIFO,
	EDF
}	t_shedule;

typedef struct parsingv_s
{
	unsigned int	n_cod;
	unsigned int	burnout_time;
	unsigned int	compile_time;
	unsigned int	debug_time;
	unsigned int	refactor_time;
	unsigned int	required_compil;
	unsigned int	dungle_time;
	t_shedule		sheduler;
}	t_pars;

t_error	parse(int argc, char **argv, t_pars *pars_values);
void	send_error(t_error error);

#endif
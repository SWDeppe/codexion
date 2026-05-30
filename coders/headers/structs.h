/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   structs.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:46:23 by sdeppe            #+#    #+#             */
/*   Updated: 2026/05/29 14:00:18 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_STRUCTS_H
# define FT_STRUCTS_H
# include "imports.h"

enum {
	WAITING,
	DUBUGING,	
	COMPILING,
	REFACTORING,
};

typedef struct s_dungle 
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long			holder; // -1 (unused) or coder index
}				t_dungle;

typedef struct s_compilation
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long			last;
}				t_compil;

typedef struct s_monitor
{
	t_compil		*last_compilations;
	pthread_t       thread;
	bool			stop;
	pthread_mutex_t stop_mutex;
}				t_monitor;

typedef struct s_coder 
{
	pthread_t		thread;
	t_dungle		left_dungle;
	t_dungle		right_dungle;
	size_t			id;
	bool			burned_out;
	bool			status;
	bool			*stop;
	pthread_mutex_t	stop_mutex;
	t_compil		*compilation;
}				t_coder;



// garbage collector
# define FREE -1

typedef struct s_pointer_data
{
	void	*ptr;
	size_t	siz;
}		t_ptr;

#endif
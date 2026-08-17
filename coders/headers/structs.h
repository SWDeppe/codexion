/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   structs.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:46:23 by sdeppe            #+#    #+#             */
/*   Updated: 2026/05/30 20:32:39 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_STRUCTS_H
# define FT_STRUCTS_H
# include "imports.h"

enum {
	WAITING,
	DEBUGING,	
	COMPILING,
	REFACTORING,
};

enum {
	FIFO, // FIRST IN FIRST OUT
	EDF   // EARLIEST DEADLINE FIRST
};

typedef struct s_dungle 
{
	unsigned int	id;
	unsigned int	next_holder;
	long			last_use;
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

typedef struct s_queue
{
	unsigned int	queue_type;
	t_dungle		*dungles;
}			 	t_queue;

typedef struct s_coder
{
	pthread_t		thread;
	t_dungle		*left_dungle;
	t_dungle		*right_dungle;
	size_t			id;
	unsigned int	status;
	long			last_start_time;
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
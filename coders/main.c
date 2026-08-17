/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:43:25 by sdeppe            #+#    #+#             */
/*   Updated: 2026/05/30 20:23:01 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"
#include "structs.h"

#define CODERS 10
#define COMPILING_TIME 1
#define DEBUGING_TIME 1
#define REFACTORING_TIME 2
#define PLUG_REALOAD 2
#define MAX 30

void *coder_routine(void *arg) {
	t_coder *coder = (t_coder *)arg;
	// printf("coder %zu says hello\n", coder->id);
	struct timeval tv;
	// unsigned int time = 1000000;
	while (1) {
		// strop if monitor says to
		pthread_mutex_lock(&coder->stop_mutex);
		if (*coder->stop)
		{
			printf("stoped a coder thread");
			return (NULL);
		}
		pthread_mutex_unlock(&coder->stop_mutex);
		// check if it is possible to compile
		while (coder->status == WAITING) {
			pthread_mutex_lock(&coder->left_dungle->mutex);
			pthread_mutex_lock(&coder->right_dungle->mutex);
			if (coder->left_dungle->holder == -1 
				&& coder->right_dungle->holder == -1
				&& coder->right_dungle->next_holder == coder->id
				&& coder->left_dungle->next_holder == coder->id)
			{
				printf("coder %zu took dungle %u and %u\n", coder->id, coder->right_dungle->id,  coder->left_dungle->id);
				coder->left_dungle->holder = coder->id;
				coder->right_dungle->holder = coder->id;
				coder->status = COMPILING;
				gettimeofday(&tv, NULL);
				coder->last_start_time = tv.tv_sec;
				coder->compilation->last = tv.tv_sec;
			}
			if (coder->status == WAITING) {
				printf("coder %zu waites for cond\n", coder->id);
				pthread_cond_wait(&coder->left_dungle->cond, &coder->left_dungle->mutex);
				printf("coder %zu got left dungle cond\n", coder->id);
				pthread_cond_wait(&coder->right_dungle->cond, &coder->right_dungle->mutex);
				printf("coder %zu got right dungle cond\n", coder->id);
			}
			pthread_mutex_unlock(&coder->left_dungle->mutex);
			pthread_mutex_unlock(&coder->right_dungle->mutex);
		}
		gettimeofday(&tv, NULL);
		// printf("coder %zu could out of loop", coder->id);
		if (coder->status == COMPILING
			&& tv.tv_sec - coder->last_start_time > COMPILING_TIME) {
			printf("coder %zu starts refactoring\n", coder->id);
			pthread_mutex_lock(&coder->left_dungle->mutex);
			pthread_mutex_lock(&coder->right_dungle->mutex);
			coder->left_dungle->holder = -1;
			coder->right_dungle->holder = -1;
			pthread_cond_broadcast(&coder->left_dungle->cond);
			pthread_cond_broadcast(&coder->right_dungle->cond);
			pthread_mutex_unlock(&coder->left_dungle->mutex);
			pthread_mutex_unlock(&coder->right_dungle->mutex);
			coder->last_start_time = tv.tv_sec;
			coder->status = REFACTORING;
		}
		else if (coder->status == REFACTORING
			&& tv.tv_sec - coder->last_start_time > REFACTORING_TIME) {
			coder->last_start_time = tv.tv_sec;
			coder->status = DEBUGING;
		}
		else if (coder->status == DEBUGING
			&& tv.tv_sec - coder->last_start_time > DEBUGING_TIME) {
			coder->last_start_time = tv.tv_sec;
			coder->status = WAITING;
		}
	}
}

void *monitor_routine(void *arg) {
	t_monitor *monitor = (t_monitor *)arg;
	struct timeval tv;
	while (1) {
		gettimeofday(&tv, NULL);
		long now = tv.tv_sec;
		int i = 0;
		while (i < CODERS) {
			pthread_mutex_lock(&monitor->last_compilations[i].mutex);
			if (now - monitor->last_compilations[i].last > MAX)
			{
				pthread_mutex_lock(&monitor->stop_mutex);
				monitor->stop = true;
				pthread_mutex_unlock(&monitor->stop_mutex);
				printf("coder %i died\n", i);
				return NULL;
			}
			pthread_mutex_unlock(&monitor->last_compilations[i].mutex);
			
			i++;
		}
		usleep(30);
	}
	
	return NULL;
}

int main() {
	// parse the elements
		// 	TODO
	// initialize the datastructure
	
	// CODERS > 0 
	if (CODERS <= 0)
		return 1; // unexpected usage
	
	/*				DUNGLES 			*/
	// alloc
	t_dungle	*dungles;
	dungles = (t_dungle *)gc_malloc(sizeof(t_dungle) * CODERS, NULL);

	// set default
	int i = 0;
	while (i < CODERS) {
		dungles[i].holder = -1;
		dungles[i].id = i;
		pthread_mutex_init(&dungles[i].mutex,
							NULL);
		pthread_cond_init(&dungles[i].cond,
							NULL);
		i++;
	}
	
	/*				 MONITOR	 			*/
	t_monitor		monitor;
	monitor.last_compilations = (t_compil *)gc_malloc(
		sizeof(t_compil) * CODERS,
		NULL
	);
	
	monitor.stop = false;
	pthread_mutex_init(&monitor.stop_mutex,
						NULL);
	
	pthread_create(&monitor.thread, NULL, &monitor_routine, &monitor);

	struct timeval tv;
	i = 0;
	gettimeofday(&tv, NULL);
	while (i < CODERS) {
		monitor.last_compilations[i].last = tv.tv_sec;
		pthread_mutex_init(&monitor.last_compilations[i].mutex,
							NULL);
		i++;
	}
	
	/*				 CODERS	 			*/
	// alloc
	t_coder		*coders;
	coders = (t_coder *)gc_malloc(sizeof(t_coder) * CODERS, NULL);
	if (coders == NULL)
		return 2; // alloc error

	// set dungles
	i = 0;
	while (i < CODERS) {
		coders[i].left_dungle = &dungles[i];
		if (i == CODERS - 1)
			coders[i].right_dungle = &dungles[0];
		else
			coders[i].right_dungle = &dungles[i + 1];
		coders[i].id = i;
		coders[i].status = WAITING;
		coders[i].compilation = &monitor.last_compilations[i];
		coders[i].stop = &monitor.stop;
		// set the threads
		pthread_create(&coders[i].thread, NULL, &coder_routine, &coders[i]);
		i++;
	}

	
	/*				 THREADS	 			*/
	// set the monitor
	
	pthread_join(monitor.thread, NULL);

	i = 0;
	while (i < CODERS) {
		pthread_join(coders[i].thread, NULL);
		i++;
	}
	gc_malloc(FREE, NULL);
}


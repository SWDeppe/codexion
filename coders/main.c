/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:43:25 by sdeppe            #+#    #+#             */
/*   Updated: 2026/05/30 14:56:35 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "functions.h"
#include "structs.h"

#define CODERS 10
#define COMPILING_TIME 200000
#define DEBUGING_TIME 20000
#define REFACTORING_TIME 30000
#define MAX 3

void *coder_routine(void *arg) {
	t_coder *coder = (t_coder *)arg;
	printf("coder %zu says hello\n", coder->id);
	struct timeval tv;
	unsigned int time = 1000000;
	while (1) {
		coder->status = WAITING;
		// strop if monitor says to
		pthread_mutex_lock(&coder->stop_mutex);
		if (*coder->stop)
		{
			printf("stoped a coder thread");
			return (NULL);
		}
		pthread_mutex_unlock(&coder->stop_mutex);
		// check if it is possible to compile
		pthread_mutex_lock(&coder->left_dungle.mutex);
		pthread_mutex_lock(&coder->right_dungle.mutex);
		if (coder->left_dungle.holder == -1 &&
			coder->right_dungle.holder == -1)
		{
			coder->left_dungle.holder = coder->id;
			coder->right_dungle.holder = coder->id;
			coder->status = COMPILING;
		}
		pthread_mutex_unlock(&coder->compilation->mutex);
		pthread_mutex_unlock(&coder->compilation->mutex);
		if (coder->status == COMPILING) {
			usleep(COMPILING_TIME);
		} else {
			// choose between refactor or dubuging
			
		}
		pthread_mutex_lock(&coder->compilation->mutex);
		gettimeofday(&tv, NULL);
		coder->compilation->last = tv.tv_sec;
		pthread_mutex_unlock(&coder->compilation->mutex);
		
		
		
		printf("coder %zu set his last compilation to %li\n", coder->id, coder->compilation->last);
		usleep(time);
		time += 1000000;
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
				printf("someone died\n");
				return NULL;
			}
			pthread_mutex_unlock(&monitor->last_compilations[i].mutex);
			
			i++;
			usleep(30);
		}
		
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
		coders[i].left_dungle = dungles[i];
		if (i == CODERS - 1)
			coders[i].right_dungle = dungles[0];
		else
			coders[i].right_dungle = dungles[i + 1];
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


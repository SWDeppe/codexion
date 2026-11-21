/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_use_dungles.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdeppe <sdeppe@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/13 05:27:22 by sdeppe            #+#    #+#             */
/*   Updated: 2026/11/13 05:32:39 by sdeppe           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_error	broadcast_wake(pthread_mutex_t *mtx, pthread_cond_t *cond)
{
	if (pthread_mutex_lock(mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_cond_broadcast(cond) != SUCCESS)
		return (PTHREAD_ERROR);
	if (pthread_mutex_unlock(mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	set_times_to_broadcast(void **args)
{
	t_coder		*coder;
	uint64_t	*ms_time;

	coder = (t_coder *)args[0];
	ms_time = (uint64_t *)args[1];
	run("Coder", get_time, (void **)&(t_get_tm){ms_time});
	if (pthread_mutex_lock(&coder->compile_done_time_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	coder->compile_done_time = *ms_time;
	if (pthread_mutex_unlock(&coder->compile_done_time_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	if (broadcast_wake(coder->wake_monit_mtx, coder->wake_monit_cond)
		!= SUCCESS)
		return (PTHREAD_ERROR);
	if (precise_sleep(coder->waiting_times[COMPILING]) != SUCCESS)
		return (GET_TIME_OF_DAY_ERROR);
	if (broadcast_wake(coder->wake_shedu_mtx, coder->wake_shedu_cond)
		!= SUCCESS)
		return (PTHREAD_ERROR);
	return (SUCCESS);
}

static t_error	free_dung_and_refactor(void **args)
{
	t_coder		*coder;
	uint64_t	*ms_time;

	coder = (t_coder *)args[0];
	ms_time = (uint64_t *)args[1];
	if (pthread_mutex_lock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	coder->can_take_dungles = NOT_ASKING;
	if (pthread_mutex_unlock(&coder->can_take_mtx) != SUCCESS)
		return (PTHREAD_ERROR);
	run("Coder", get_time, (void **)&(t_get_tm){ms_time});
	print("%lu coder %u is refacotring\n", (uint32_t []){
		*ms_time - *coder->start, coder->id}, 2);
	coder->used_dungle[0] = (uint32_t)DUNGLE_UNUSED;
	coder->used_dungle[1] = (uint32_t)DUNGLE_UNUSED;
	if (precise_sleep(coder->waiting_times[REFACTORING]) != SUCCESS)
		return (GET_TIME_OF_DAY_ERROR);
	return (SUCCESS);
}

static void	print_access(t_coder *coder, uint64_t ms_time)
{
	print("%li coder %u got access to dungle %i !\n", (uint32_t []){
		ms_time - *coder->start, coder->id, coder->dungles[0]}, 3);
	print("%li coder %u got access to dungle %i !\n", (uint32_t []){
		ms_time - *coder->start, coder->id, coder->dungles[1]}, 3);
	print("%li coder %u is Compiling\n", (uint32_t []){
		ms_time - *coder->start, coder->id}, 2);
}

int	use_dungles(t_coder *coder, uint64_t ms_time)
{
	uint8_t	stop;

	stop = 0;
	print_access(coder, ms_time);
	coder->compile_count++;
	run("Coder", set_times_to_broadcast, (void *[]){coder, &ms_time});
	run("Coder", error_check, (void *[]){coder, &stop});
	if (stop)
		return (1);
	run("Coder", free_dung_and_refactor, (void *[]){coder, &ms_time});
	run("Coder", error_check, (void *[]){coder, &stop});
	if (stop)
		return (1);
	run("Coder", get_time, (void **)&(t_get_tm){&ms_time});
	print("%lu coder %u is Debuging\n", (uint32_t []){
		ms_time - *coder->start, coder->id}, 2);
	if (precise_sleep(coder->waiting_times[DEBUGING]) != SUCCESS)
		return (1);
	run("Coder", error_check, (void *[]){coder, &stop});
	if (stop)
		return (1);
	return (0);
}

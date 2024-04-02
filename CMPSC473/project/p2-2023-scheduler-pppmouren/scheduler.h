/*
 * Utilize "scheduler.h" and "scheduler.c" for all the utility functions students
 * intend to use for "interface.c".
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>

#include "interface.h"

#endif

typedef struct thread_info
{
    int tid;
    int remain_time;
    int arrive_time;
    pthread_cond_t cv;
    pthread_cond_t io_cv;
    struct thread_info *next; 
} thread_info_t;

typedef struct Linked_list
{
    thread_info_t *head;
    thread_info_t *tail;
}Linked_list_t;

//add the thread entry into linked list and return current number of thread that into the list
bool add_thread_to_fifo_list(float arrive_time, int tid, int device_id, Linked_list_t *list);

//add the thread entry into linked list in srtf way and return current number of thread in list
int add_thread_to_srtf_list(float arrive_time, int tid, int remain_time, Linked_list_t *list, int curr_num_thread);

//find the next thread to schedule by srtf
thread_info_t* find_next_thread(Linked_list_t *list, int clocks);

//update arrive time for those threads with arrive time less or equal to clocks. Update them to clocks
void update_arrive_time(Linked_list_t *list, int clocks);

//return the thread entry in the linked list based on provided tid
thread_info_t* get_self_thread(Linked_list_t *linked_list, int tid);

//kick the first entry in the linked list and return the new current number of thread
int pop_thread_from_list(Linked_list_t *list, int curr_num_thread);

//get the ceil 
int get_ceiling (float arrive_time);

//remove thread from list, could be anywhere and return new current number of thread
int remove_thread_from_srtf(Linked_list_t *list, int curr_num_thread, thread_info_t *thread);
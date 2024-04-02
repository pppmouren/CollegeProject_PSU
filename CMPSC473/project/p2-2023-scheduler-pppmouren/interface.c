#include "interface.h"
#include "scheduler.h"

// Interface implementation
int clocks;
Linked_list_t *IO1_list;
Linked_list_t *srtf_list;
int num_thread;
int curr_num_thread;
int io1_return_time;
int io_num_thread;
pthread_mutex_t num_mutex;
pthread_mutex_t cpu_mutex;
pthread_mutex_t io1_mutex;
pthread_mutex_t clock_mutex;
thread_info_t *srtf_next_thread; 
int flag = 0;

void init_scheduler(int thread_count) {
    // TODO: Implement this
    clocks = 0;
    num_thread = thread_count;
    curr_num_thread = 0;
    io1_return_time = 0;
    io_num_thread = 0;
    IO1_list = (Linked_list_t*)malloc(sizeof(Linked_list_t));
    IO1_list->head = NULL;
    IO1_list->tail = NULL;
    srtf_list = (Linked_list_t*)malloc(sizeof(Linked_list_t));
    srtf_list->head = NULL;
    srtf_list->tail = NULL;
    pthread_mutex_init(&num_mutex, NULL);
    pthread_mutex_init(&cpu_mutex, NULL);
    pthread_mutex_init(&io1_mutex, NULL);
    pthread_mutex_init(&clock_mutex, NULL);
    srtf_next_thread = NULL;
    flag = 0;
}

int cpu_me(float current_time, int tid, int remaining_time) {
    // TODO: Implement this
    //insert thread into linked_list until all the threads comes in
    //go to mutex lock
    pthread_mutex_lock(&num_mutex);

    curr_num_thread = add_thread_to_srtf_list(current_time,tid, remaining_time,srtf_list,curr_num_thread);
    while(curr_num_thread != num_thread){
        pthread_cond_wait(&get_self_thread(srtf_list, tid)->cv, &num_mutex);
    }
    pthread_mutex_unlock(&num_mutex);
    //the thread gets out will be the last thread go to the list around all lists
    //going to signal the thread to go for cpu and io
    pthread_mutex_lock(&cpu_mutex);

    if(IO1_list->head != NULL){
        pthread_cond_signal(&IO1_list->head->cv);
    }
    srtf_next_thread = find_next_thread(srtf_list, clocks);
    if(srtf_next_thread != NULL){
        pthread_cond_signal(&srtf_next_thread->cv);
    }
    //check condition
    while(srtf_next_thread->tid != tid){
        pthread_cond_wait(&get_self_thread(srtf_list, tid)->cv, &cpu_mutex);
    }
    //pthread_mutex_unlock(&cpu_mutex);

    pthread_mutex_lock(&clock_mutex);
    //increment clock time
    if(remaining_time > 0){
        int sche_time = get_ceiling(current_time);
        if (clocks < sche_time){
            clocks = sche_time + 1;
        }
        else{
            clocks = clocks + 1;
        }
        srtf_next_thread->remain_time -= 1;

        if(clocks >= io1_return_time && io1_return_time != 0 && IO1_list->head != NULL){
            pthread_cond_signal(&IO1_list->head->io_cv);
        }

    }
    pthread_mutex_unlock(&clock_mutex);

    //kick out the first entry when the remaining time is 0
    if(remaining_time == 0){
        curr_num_thread = remove_thread_from_srtf(srtf_list, curr_num_thread, get_self_thread(srtf_list, tid));
    }

    //release the lock
    pthread_mutex_unlock(&cpu_mutex);
    return clocks;
}

int io_me(float current_time, int tid, int device_id) {
    // TODO: Implement th
    pthread_mutex_lock(&num_mutex);
    flag = 1;
    if(add_thread_to_fifo_list(current_time,tid, device_id, IO1_list) == true){
        curr_num_thread ++;
        io_num_thread ++;
    }
    while(curr_num_thread != num_thread){
        pthread_cond_wait(&get_self_thread(IO1_list, tid)->cv, &num_mutex);
    }
    pthread_mutex_unlock(&num_mutex);

    //the thread gets out will be the last thread go to the list around all lists
    //going to signal the thread to go for cpu and io
    pthread_mutex_lock(&io1_mutex);
    if(IO1_list->head != NULL){
        pthread_cond_signal(&IO1_list->head->cv);
    }
    srtf_next_thread = find_next_thread(srtf_list, clocks);
    if(srtf_next_thread != NULL){
        pthread_cond_signal(&srtf_next_thread->cv);
    }

    //check condition
    while(tid != IO1_list->head->tid){
        pthread_cond_wait(&get_self_thread(IO1_list, tid)->cv, &cpu_mutex);
    }

    pthread_mutex_lock(&clock_mutex);
    int sche_time = get_ceiling(current_time);
    if(io_num_thread == num_thread){
        if (clocks < sche_time){
            clocks = sche_time + IO_DEVICE_1_TICKS;
            io1_return_time = clocks;
        }
        else{
            clocks = clocks + IO_DEVICE_1_TICKS;
            io1_return_time = clocks;
        }
    }
    else{
        if (clocks < sche_time){
            clocks = sche_time;
        }
        io1_return_time = IO_DEVICE_1_TICKS + clocks;
        while(io1_return_time > clocks){
            //pthread_cond_wait(&get_self_thread(IO1_list, tid)->cv, &clock_mutex);
            pthread_cond_wait(&get_self_thread(IO1_list, tid)->io_cv, &clock_mutex);
        }
    }
    pthread_mutex_unlock(&clock_mutex);
    flag = 0;

    //kick out the first entry when the remaining time is 0
    curr_num_thread = pop_thread_from_list(IO1_list, curr_num_thread);
    io_num_thread --;
    
    pthread_mutex_unlock(&io1_mutex);
    return io1_return_time;
}

void end_me(int tid) {
    // TODO: Implement this
    //notify the fifo scheduler that there is one thread say bye-bye, and signal the next thread
    pthread_mutex_lock(&num_mutex);

    num_thread -= 1;
    //signal next signal
    if(IO1_list->head != NULL){
        pthread_cond_signal(&IO1_list->head->cv);
    }
    srtf_next_thread = find_next_thread(srtf_list, clocks);
    if(srtf_next_thread != NULL){
        pthread_cond_signal(&srtf_next_thread->cv);
    }
    pthread_mutex_unlock(&num_mutex);

    // srtf_next_thread = find_next_thread(srtf_list, clocks);
    // if(srtf_next_thread != NULL){
    //     pthread_cond_signal(&srtf_next_thread->cv);
    // }
    
    
}



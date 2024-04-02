#include "scheduler.h"

// TODO: Add your function definitions here.
thread_info_t* get_self_thread(Linked_list_t *linked_list, int tid){
    thread_info_t *curr = linked_list->head;
    while(1){
        if (curr->tid == tid){
            return curr;
        }
        curr = curr->next;
        if(curr == NULL){
            break;
        }
    }
    return NULL;
}

bool add_thread_to_fifo_list(float arrive_time, int tid, int device_id, Linked_list_t *list){
    thread_info_t *new_thread = (thread_info_t*) malloc(sizeof(thread_info_t));
    new_thread->tid = tid;
    if(device_id == 1){
    new_thread->remain_time = 2;
    }
    else{
        new_thread->remain_time = 5;
    }
    new_thread->arrive_time = arrive_time;
    pthread_cond_init(&new_thread->cv, NULL);
    pthread_cond_init(&new_thread->io_cv, NULL);
    new_thread->next = NULL;

    //create curr an dprev to locate where the new thread should put
    //easier to determine siuation and put thread
    thread_info_t *curr_thread = list->head;
    thread_info_t *prev_thread = NULL;
    thread_info_t *check_thread = list->head;

    //check if the thread is in list or not
    while(check_thread != NULL){
        if(check_thread->tid == tid){
            return false;
        }
        check_thread = check_thread->next;
    }


    //put first entry
    if(list->head == NULL){
        list->head = new_thread;
        list->tail = new_thread;
    }
    else{
        //loop through the linked list, to find the node where the new thread need to put
        while(curr_thread != NULL && curr_thread->arrive_time < arrive_time){
            prev_thread = curr_thread;
            curr_thread = curr_thread->next;
        }
        while(curr_thread != NULL && curr_thread->arrive_time == arrive_time && curr_thread->tid < tid){
            prev_thread = curr_thread;
            curr_thread = curr_thread->next;
        }
    }
    //add the new thread node to the linked list
    if(prev_thread == NULL){
        //new thread should add as head thread
        new_thread->next = list->head;
        list->head = new_thread;
    }
    else{
        new_thread->next = prev_thread->next;
        prev_thread->next = new_thread;
        //when new thread add as a tail
        if(curr_thread == NULL){
            list->tail = new_thread;
        }
    }
    return true;
}

int add_thread_to_srtf_list(float arrive_time, int tid, int remain_time, Linked_list_t *list, int curr_num_thread){
    thread_info_t *new_thread = (thread_info_t*) malloc(sizeof(thread_info_t));
    new_thread->tid = tid;
    new_thread->remain_time = remain_time;
    new_thread->arrive_time = get_ceiling(arrive_time);
    pthread_cond_init(&new_thread->cv, NULL);
    pthread_cond_init(&new_thread->io_cv, NULL);
    new_thread->next = NULL;

    //create curr an dprev to locate where the new thread should put
    //easier to determine siuation and put thread
    thread_info_t *curr_thread = list->head;
    thread_info_t *prev_thread = NULL;
    thread_info_t *check_thread = list->head;

    //check if the thread is in list or not
    while(check_thread != NULL){
        if(check_thread->tid == tid){
            return curr_num_thread;
        }
        check_thread = check_thread->next;
    }

    //put the first entry to list if list is enpty
    if(list->head == NULL){
        list->head = curr_thread;
        list->tail = curr_thread;
    }
    //find the exact location the new thread will add
    else{
        while(curr_thread != NULL && curr_thread->arrive_time < arrive_time){
            prev_thread = curr_thread;
            curr_thread = curr_thread->next;
        }
        while(curr_thread != NULL && curr_thread->arrive_time == arrive_time && curr_thread->remain_time < remain_time){
            prev_thread = curr_thread;
            curr_thread = curr_thread->next;
        }
        while(curr_thread != NULL && curr_thread->arrive_time == arrive_time && curr_thread->remain_time == remain_time && curr_thread->tid < tid){
            prev_thread = curr_thread;
            curr_thread = curr_thread->next;
        }
    }

    //add the thread to list now
    if(prev_thread == NULL){
        //new thread should add as head thread
        new_thread->next = list->head;
        list->head = new_thread;
    }
    else{
        new_thread->next = prev_thread->next;
        prev_thread->next = new_thread;

        //if new thread is added as tail
        if(curr_thread == NULL){
            list->tail = new_thread;
        }
    }
    curr_num_thread ++;
    return curr_num_thread;
}

thread_info_t* find_next_thread(Linked_list_t *list, int clocks){
    thread_info_t *temp = list->head;
    thread_info_t *ret_thread = list->head;
    while(temp != NULL && temp->arrive_time <= clocks){
        if(temp->remain_time < ret_thread->remain_time){
            ret_thread = temp;
        }
        temp = temp->next;
    }
    return ret_thread;
}

void update_arrive_time(Linked_list_t *list, int clocks){
    thread_info_t *curr_thread = list->head;
    if(list->head != NULL){
        //update all the threads' arrive time = clocks if arrive time is less than clocks
        while (curr_thread != NULL && curr_thread->arrive_time <= clocks){
            curr_thread->arrive_time = clocks;
            curr_thread = curr_thread->next;
        }
    }
}

int pop_thread_from_list(Linked_list_t *list, int curr_num_thread){
    //pop the head of the list
    thread_info_t *temp = list->head;
    //if only one thread in the list
    if(list->head == list->tail){
        list->head = NULL;
        list->tail = NULL;
    }
    //more than one thread in list
    else{
        list->head = list->head->next;
    }
    free(temp);
    curr_num_thread -= 1;
    return curr_num_thread;
}

int remove_thread_from_srtf(Linked_list_t *list, int curr_num_thread, thread_info_t *thread){
    thread_info_t *prev = list->head;
    //if there is only one thread in list
    if(list->head == list->tail){
        list->head = NULL;
        list->tail = NULL;
    }
    else{
        //if remove head
        if(thread == list->head){
            list->head = list->head->next;
        }
        else{
            //get the prev thread before this thread
            while(prev->next != thread){
                prev = prev->next;
            }
            //remove this thread
            prev->next = thread->next;
            thread->next = NULL;
            //if removed thread is tail then, set the new tail as prev
            if(thread == list->tail){
                list->tail = prev;
            }
        }
    }
    free(thread);
    curr_num_thread -= 1;
    return curr_num_thread;
}

int get_ceiling (float arrive_time){
    int ret = (int)arrive_time;
    if ((float)ret == arrive_time){
        return ret;
    }
    return ret + 1;
}


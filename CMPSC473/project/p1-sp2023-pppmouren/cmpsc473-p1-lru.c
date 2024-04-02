
/**********************************************************************

   File          : cmpsc473-p1-lru.c

   Description   : This is LRU page replacement algorithm

   Last Modified : Jan 11 09:54:33 EST 2023
   By            : Trent Jaeger

***********************************************************************/
/**********************************************************************
Copyright (c) 2023 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this softwiare without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

/* Include Files */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>

/* Project Include Files */
#include "cmpsc473-p1.h"

/* Definitions */

/* lru list */

typedef struct lru_entry {  
  unsigned int pid;
  ptentry_t *ptentry;
  struct lru_entry *next;
  struct lru_entry *prev;
} lru_entry_t;

typedef struct lru {
  lru_entry_t *first;
} lru_t;

lru_t *lru_frame_list;

/**********************************************************************

    Function    : init_lru
    Description : initialize lru list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_lru( FILE *fp )
{
  printf("initiate lru...\n");
  lru_frame_list = (lru_t *)malloc(sizeof(lru_t));
  lru_frame_list->first = NULL;
  return 0;
}


/**********************************************************************

    Function    : replace_lru
    Description : choose victim from lru list (first in list is oldest)
    Inputs      : victim - process id of victim frame 
                  frame - frame assigned from lru replacement
                  ptentry - pt entry mapping frame currently -- to be invalidated
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_lru( unsigned int *victim, frame_t **frame, ptentry_t **ptentry )
{
  /* Task 3(b) */
  //aging all the lru first
  lru_entry_t *aging_curr_entry = lru_frame_list->first;
  while (TRUE)
  {
    if (aging_curr_entry->ptentry->ref == 0){
      physical_mem[aging_curr_entry->ptentry->frame].lru = physical_mem[aging_curr_entry->ptentry->frame].lru >> 1;
      //printf("current node: %p; frame number: %d; current ptentry ref = 0; updated lru = %d \n", aging_curr_entry,aging_curr_entry->ptentry->frame, physical_mem[aging_curr_entry->ptentry->frame].lru);
    }
    else{
      physical_mem[aging_curr_entry->ptentry->frame].lru = (physical_mem[aging_curr_entry->ptentry->frame].lru >> 1) | 0b100;
      aging_curr_entry->ptentry->ref = 0;
      //printf("current node: %p; frame number: %d; current ptentry ref = 1; updated lru = %d \n", aging_curr_entry,aging_curr_entry->ptentry->frame, physical_mem[aging_curr_entry->ptentry->frame].lru);
    }
    aging_curr_entry = aging_curr_entry->next;
    if (aging_curr_entry == lru_frame_list->first){
      break;
    }
  }

  //find the node with the first samllest lru field
  lru_entry_t *vict_entry = lru_frame_list->first;
  while(TRUE){
    if (physical_mem[vict_entry->ptentry->frame].lru > physical_mem[aging_curr_entry->ptentry->frame].lru){
      vict_entry = aging_curr_entry;
    }
    //printf("after compare ref, current vict entry is %p; compared aging_entry is %p \n", vict_entry, aging_curr_entry);
    aging_curr_entry = aging_curr_entry->next;
    //printf("next aging node is %p \n", aging_curr_entry);
    if(aging_curr_entry == lru_frame_list->first){
      break;
    }
  }
  //printf("\n the vict entry is %p; and frame number is: %d\n", vict_entry, vict_entry->ptentry->frame);
  //return info on victim
  *victim = vict_entry->pid;
  *ptentry = vict_entry->ptentry;
  *frame = &physical_mem[vict_entry->ptentry->frame];

  //remove from the list
  lru_frame_list->first = vict_entry->next;
  lru_entry_t *prev_entry = vict_entry->prev;
  prev_entry->next =lru_frame_list->first;
  lru_frame_list->first->prev = prev_entry;
  free(vict_entry);
  return 0;
}


/**********************************************************************

    Function    : update_lru
    Description : update lru list on allocation (add entry to end)
    Inputs      : pid - process id
                  ptentry - mapped to frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_lru( unsigned int pid, ptentry_t *ptentry)
{
  /* Task 3(b) */
  //make a new entry
  lru_entry_t *lru_list_entry = (lru_entry_t*)malloc(sizeof(lru_entry_t));
  lru_list_entry->pid = pid;
  lru_list_entry->ptentry = ptentry;
  lru_list_entry->next = lru_list_entry;
  lru_list_entry->prev = lru_list_entry;

  //fram list elements start with lru counter bit as 0b100;
  unsigned int fnum = lru_list_entry->ptentry->frame;
  physical_mem[fnum].lru = 4;
  
  //reset ref bit back to 0
  lru_list_entry->ptentry->ref = 0;

  //we want to put the new entry into the page list
  //if the page list is empty
  if(lru_frame_list->first == NULL){
    lru_frame_list->first = lru_list_entry;
  }

  //or we add to the page list to the end (add before the where the first pionts to)
  else{
    lru_entry_t *prev_entry = lru_frame_list->first->prev;
    lru_list_entry->next = lru_frame_list->first;
    lru_frame_list->first->prev = lru_list_entry;
    lru_list_entry->prev = prev_entry;
    prev_entry->next = lru_list_entry;
  }
  return 0;  
}



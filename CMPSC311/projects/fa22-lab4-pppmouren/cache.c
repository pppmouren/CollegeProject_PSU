#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int num_queries = 0;
static int num_hits = 0;
int is_created = 0;

//check if the copprsponding block and disk number in cache
//return -1 for not in the cache, o.w. return the index
int is_in_cache(int block_id, int disk_id, cache_entry_t *cache){
  for(int i = 0; i < cache_size; i++){
    if((block_id == cache[i].block_num) && (disk_id == cache[i].disk_num)){
      return i;
    }
  }
  return -1;
}

//find where to replace based on LFU policy
int get_index(cache_entry_t *cache){
  int index = 0;
  for(int i = 1; i < cache_size; i++){
    if (cache[index].num_accesses > cache[i].num_accesses){
      index = i;
    }
  }
  return index;
}

//create an cache entry for update/insert
cache_entry_t insert_update_entry(int block_id, int disk_id, const uint8_t *buffer){
  cache_entry_t one_entry;
  one_entry.valid = true;
  one_entry.disk_num = disk_id;
  one_entry.block_num = block_id;
  memcpy(one_entry.block,buffer,JBOD_BLOCK_SIZE);
  one_entry.num_accesses = 1;
  return one_entry;
}

int cache_create(int num_entries) {
  if ((is_created == 1) || (num_entries < 2) || (num_entries > 4096)){
    return -1;
  }
  cache = calloc(num_entries, sizeof(cache_entry_t));
  cache_size = num_entries;
  is_created = 1;
  return 1;
}

int cache_destroy(void) {
  if (is_created == 0){
    return -1;
  }
  
  //deallocated space
  free(cache);
  cache = NULL;
  cache_size = 0;
  is_created = 0;
  return 1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  //chech parameter
  int i = is_in_cache(block_num, disk_num, cache);
  if((is_created == 0) || (buf == NULL) || (i == -1) || (cache[i].valid == false) ){
    num_queries++;
    return -1;
  }
  else{
    //copy the block into buffer
    memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
  
    //update variables
    num_queries++;
    num_hits++;
    cache[i].num_accesses++;
    return 1;
  }
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  //find index
  int i = is_in_cache(block_num, disk_num, cache);

  //updating new entry
  if((i != -1) && (cache[i].valid == true)){
    cache[i] = insert_update_entry(block_num, disk_num, buf);
  }
  
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  //chech parameter
  int i = is_in_cache(block_num, disk_num, cache);
  if((buf == NULL) || ((i != -1) && (cache[i].valid == true))  || (is_created == 0) || (disk_num < 0) || (disk_num > JBOD_NUM_DISKS - 1) || (block_num < 0) || (block_num > JBOD_NUM_BLOCKS_PER_DISK - 1 )){
    return -1;
  }

  //inserting new entry
  int index = get_index(cache);
  cache[index] = insert_update_entry(block_num, disk_num, buf);
  return 1;
}

bool cache_enabled(void) {
	return cache != NULL && cache_size > 0;
}

void cache_print_hit_rate(void) {
	fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
	fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}

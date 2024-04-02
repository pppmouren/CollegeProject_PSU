#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "jbod.h"
#include "mdadm.h"

int is_mounted = 0;
int is_written = 0;

//a function that calculate op//	     
uint32_t build_op(uint32_t blockID, uint32_t diskID, uint32_t command){
  uint32_t result = (blockID) | (diskID << 8) | (command << 12);
  return result;
}

//a function that use to seek disk and block
void seek_func(uint32_t seektoblock_op, uint32_t seektodisk_op){
  jbod_operation(seektodisk_op, NULL);
  jbod_operation(seektoblock_op, NULL);
}

int mdadm_mount(void) {
  //get the right mount_op//
  uint32_t mount_op = build_op(0, 0, JBOD_MOUNT );
  
  //check if mount is successful//
  if (jbod_operation(mount_op,NULL) == 0){
    is_mounted = 1;
    return 1;
  }
  else{
    return -1;
  }
}

int mdadm_unmount(void) {
  //get the right unmount_op
  uint32_t unmount_op = build_op(0, 0, JBOD_UNMOUNT);
  
  //check if unmount is working//
  if (jbod_operation(unmount_op,NULL) == 0){
    is_mounted = 0;
    return 1;
  }
  else{
    return -1;
  }
}

int mdadm_write_permission(void){
  //get write_permission_op
  uint32_t write_permission_op = build_op(0, 0, JBOD_WRITE_PERMISSION);

  //check if write permission working
  if(jbod_operation(write_permission_op, NULL) == 0){
     is_written = 1;
     return 1;
  }
  else{
    return -1;
  }
}


int mdadm_revoke_write_permission(void){
  //get revoke_write_permission_op
  uint32_t revoke_write_permission_op = build_op(0, 0, JBOD_REVOKE_WRITE_PERMISSION);

  //check if write permission working
  if(jbod_operation(revoke_write_permission_op, NULL) == 0){
     is_written = 0;
     return 1;
  }
  else{
    return -1;
  }
}


int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
  uint32_t read_buf_flag = 0; //track where to put temp buffer info
  uint32_t disk_flag = start_addr/JBOD_DISK_SIZE; //keep the info of current disk
  uint32_t block_flag = (start_addr/JBOD_BLOCK_SIZE)%JBOD_BLOCK_SIZE; //keep the info of current block
  uint32_t byte_addr = start_addr%JBOD_BLOCK_SIZE; //knowing the starting byte inside the current block
  uint32_t remain_len = read_len; //keep the remaining length info that need to be read
  int size = 0; //size keeps how many value need to be copied
  
  //check parameter//
  if ((is_mounted == 0) || (start_addr + read_len > 1048576) || (read_len > 2048) || ((read_len != 0) && (read_buf == NULL))){
    return -1;
  }
  
  //works for read length is 0 and buffer is null//
  else if ((read_len == 0) && (read_buf == NULL)){
    return 0;
  }
  
  //start copy
  //create op and seek to disk and block//
  uint32_t read_op = build_op(0, 0, JBOD_READ_BLOCK);
  uint32_t seek_disk_op = 0;
  uint32_t seek_block_op = 0;
    
  //allocate space in heap
  uint8_t *temp_buf = malloc(JBOD_BLOCK_SIZE);
    
  while (remain_len > 0){
    //seek block and disk
    seek_disk_op = build_op(0, disk_flag, JBOD_SEEK_TO_DISK);
    seek_block_op = build_op(block_flag, 0, JBOD_SEEK_TO_BLOCK);
    seek_func(seek_block_op, seek_disk_op);
     
    //read block
    if(cache_enabled() == true){
      int a = cache_lookup(disk_flag, block_flag, temp_buf);
      if(a == -1){
	jbod_operation(read_op, temp_buf);
	cache_insert(disk_flag, block_flag, temp_buf);
      }
    }
    else{
      jbod_operation(read_op, temp_buf);
    }

    //read first block
    if (remain_len == read_len){
      //read within block
      if(byte_addr + read_len < JBOD_BLOCK_SIZE){
	size = remain_len;
      }
      //read across block
      else{
	size = JBOD_BLOCK_SIZE - byte_addr;
      }
    }
	
    //read the last block
    else if (remain_len <= JBOD_BLOCK_SIZE){
      byte_addr = 0;
      size = remain_len;
    }
	
    //read the whole block in the middle
    else{
      byte_addr = 0;
      size = JBOD_BLOCK_SIZE;	  
    }

    //copy
    memcpy(&read_buf[read_buf_flag], &temp_buf[byte_addr], size);
    read_buf_flag += size;
    remain_len -= size;

    //update block and disk id
    if(block_flag != JBOD_BLOCK_SIZE - 1){
      block_flag++;
    }
    else{
      block_flag = 0;
      disk_flag++;
    }
  }
  free(temp_buf);
  temp_buf = NULL;
  return read_len;
    
}



int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf) {
  uint32_t write_buf_flag = 0; //track where to put temp buffer info
  uint32_t disk_flag = start_addr/JBOD_DISK_SIZE; //keep the info of current disk
  uint32_t block_flag = (start_addr/JBOD_BLOCK_SIZE)%JBOD_BLOCK_SIZE; //keep the info of current block
  uint32_t byte_addr = start_addr%JBOD_BLOCK_SIZE; //knowing the starting byte inside the current block
  uint32_t remain_len = write_len; //keep the remaining length info that need to be write
  int size = 0; //keepshow many value need to be write
  int a; //a use to keep the successful of cache_lookup, 1 for success, -1 for fail, and 0 for cache does not enable
  
  //check parameters//
  if ((is_mounted == 0) || (is_written == 0) || (start_addr + write_len > 1048576) || (write_len > 2048) || ((write_len != 0) && (write_buf == NULL))){
    return -1;
  }
  
  //works for write length is 0 and buffer is null//
  else if ((write_len == 0) && (write_buf == NULL)){
    return 0;
  }
  
  //create op and seek to disk and block//
  uint32_t read_op = build_op(0, 0, JBOD_READ_BLOCK);
  uint32_t write_op = build_op(0, 0, JBOD_WRITE_BLOCK);
  uint32_t seek_disk_op = 0;
  uint32_t seek_block_op = 0;
 
  //create temp_buffer
  uint8_t *temp_buf = malloc(JBOD_BLOCK_SIZE);
  
  while(remain_len > 0){
    //initialize a for every loop
    a = 0;
    //seek block and disk
    seek_disk_op = build_op(0, disk_flag, JBOD_SEEK_TO_DISK);
    seek_block_op = build_op(block_flag, 0, JBOD_SEEK_TO_BLOCK);
    seek_func(seek_block_op, seek_disk_op);
    
    //read
    if(cache_enabled() == true){
      a = cache_lookup(disk_flag, block_flag, temp_buf);
      if(a == -1){
	jbod_operation(read_op, temp_buf);
      }
    }
    else{
      jbod_operation(read_op, temp_buf);
    }
   
    //write the first block
    if (remain_len == write_len){
      //write within block
      if(byte_addr + write_len < JBOD_BLOCK_SIZE){
	size = remain_len;
      }
      //write across block
      else{
	size = JBOD_BLOCK_SIZE - byte_addr;
      }
    }

    //wtite the last block
    else if (remain_len <= JBOD_BLOCK_SIZE){
      size = remain_len;
      byte_addr = 0;
    }

    //write the whole block in the middle
    else{
      size = JBOD_BLOCK_SIZE;
      byte_addr = 0;
    }

    memcpy(&temp_buf[byte_addr], &write_buf[write_buf_flag], size);
    remain_len -= size;
    write_buf_flag += size;
    
    // write mem and cache
    seek_func(seek_block_op, seek_disk_op);
    jbod_operation(write_op, temp_buf);
    if(a == 1){
      cache_update(disk_flag, block_flag, temp_buf);
    }
    else if(a == -1){
      cache_insert(disk_flag, block_flag, temp_buf);
    }
    	
    //update block and disk id
    if(block_flag != JBOD_BLOCK_SIZE - 1){
      block_flag++;
    }
    else{
      block_flag = 0;
      disk_flag++;
    }
  }
  free(temp_buf);
  temp_buf = NULL;
  return write_len;
}

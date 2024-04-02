//CMPSC 311 FA22
//LAB 2

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "mdadm.h"
#include "jbod.h"

//initialize mount and unmount op//
uint32_t mount_op = 4097;
uint32_t unmount_op = 490;


//mount flag, enable 1, unable 0//
int mount_flag = 0;

//a function that calculate op//	     
uint32_t build_op(uint32_t blockID, uint32_t diskID, uint32_t command){
  uint32_t result = (blockID) | (diskID << 8) | (command << 12);
  return result;
}

//mount the linear device//
int mdadm_mount(void) {
  //get the right mount_op//
  mount_op = build_op(0, 0, JBOD_MOUNT );
  
  //check if mount is successful//
  if (jbod_operation(mount_op,NULL) == 0){
    mount_flag = 1;
    return 1;
  }
  else{
    return -1;
  }
 }

//unmount the linear device//
int mdadm_unmount(void) {
  //get the right unmount_op
  unmount_op = build_op(0, 0, JBOD_UNMOUNT);
  
  //check if unmount is working//
  if (jbod_operation(unmount_op,NULL) == 0){
    mount_flag = 0;
    return 1;
  }
  else{
    return -1;
  }
 }

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf) {
  uint32_t read_buf_flag = 0; //track where to put temp buffer info
  uint32_t disk_flag = (start_addr/256)/256; //keep the info of current disk
  uint32_t block_flag = (start_addr/256)%256; //keep the info of current block
  uint32_t byte_addr = start_addr%256; //knowing the starting byte inside the current block
  uint32_t remain_len = read_len; //keep the remaining length info that need to be read

  
  //check mount before read//
  if (mount_flag == 0){
    return -1;
  }
  
  //check if the address is out of bound linear address//
  else if (start_addr > 1048575){
    return -1;
  }
  
  //check if read beyond the linear address//
  else if (start_addr + read_len > 1048575){
    return -1;
  }
  
  //check if read length greater than 2048//
  else if (read_len > 2048){
    return -1;
  }
  
  //check is read length = 0 and need to return things to buffer//
  else if ((read_len != 0) && (read_buf == NULL)){
    return -1;
  }
  
  //works for read length is 0 and buffer is null//
  else if ((read_len == 0) && (read_buf == NULL)){
    return 0;
  }
  
  //start copy
  else {
    //create op and seek to disk and block//
    uint32_t read_op = build_op(0, 0, JBOD_READ_BLOCK);
    uint32_t seek_disk_op = build_op(0, disk_flag, JBOD_SEEK_TO_DISK);
    uint32_t seek_block_op = build_op(block_flag, 0, JBOD_SEEK_TO_BLOCK);
    jbod_operation(seek_disk_op, NULL);
    jbod_operation(seek_block_op, NULL);
    
    //allocate space in heap
    uint8_t *temp_buf = malloc(256);
    
    while (remain_len > 0){
      //check if read_within_block
      if (byte_addr + read_len <= 255){
	
      //put read operation to jbod_operation and get the info of current disk and blcok//
      jbod_operation(read_op, temp_buf);

      //copy to read_buf
      memcpy(read_buf, &temp_buf[byte_addr], remain_len);
      remain_len = 0;
      }
      
      //read across block/disk
      else{
	//read first block
	if (remain_len == read_len){
	  //size keeps the value of how many we should read in the first block
	  int size = 256 - byte_addr;
	  
	  //read block
	  jbod_operation(read_op, temp_buf);
	  
	  //copy info to read_buf
	  memcpy(read_buf, &temp_buf[byte_addr], size);

	  //update remain_len and  read_buf_flag
	  remain_len = read_len - size;
	  read_buf_flag += size;
	}
	//read the last block
	else if (remain_len <= 256){
	  //read block
	  jbod_operation(read_op, temp_buf);

	  //copy to read_buf
	  memcpy(&read_buf[read_buf_flag], temp_buf, remain_len);

	  //update remain_len
	  remain_len = 0;
	}
	//read the whole block
	else{
	  //read block
	  jbod_operation(read_op, temp_buf);

	  //copy to read_buf
	  memcpy(&read_buf[read_buf_flag], temp_buf, 256);

	  //update remain_len and read_buf_flag
	  remain_len -= 256;
	  read_buf_flag += 256;	  
	}
      }

      if (block_flag != 255){
	block_flag++;
      }
      //read across the disk, need to reseek
      else{
	block_flag = 0;
	disk_flag++;
	seek_disk_op = build_op(0, disk_flag, JBOD_SEEK_TO_DISK);
	seek_block_op = build_op(block_flag, 0, JBOD_SEEK_TO_BLOCK);
	jbod_operation(seek_disk_op, NULL);
	jbod_operation(seek_block_op, NULL);	
      }
    }
    free(temp_buf);
    return read_len;
  }
}
    











    

    
    



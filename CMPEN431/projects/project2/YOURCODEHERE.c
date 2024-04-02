#include "YOURCODEHERE.h"
#include <math.h>
/**********************************************************************
    Function    : lg2pow2
    Description : this help funciton for you to calculate the bit number
                  this function is not allowed to modify
    Input       : pow2 - for example, pow2 is 16
    Output      : retval - in this example, retval is 4
***********************************************************************/
unsigned int lg2pow2(uint64_t pow2){
  unsigned int retval=0;
  while(pow2 != 1 && retval < 64){
    pow2 = pow2 >> 1;
    ++retval;
  }
  return retval;
}

//function to generate bit mask for VAImask and VATmask
static unsigned long long bitmask(int num){
  unsigned long long mask = 0;
  for(int i = 0; i < num; i++){
    mask |= ((uint64_t) 1 ) << i;
  }
  return mask;
}

void  setSizesOffsetsAndMaskFields(cache* acache, unsigned int size, unsigned int assoc, unsigned int blocksize){
  unsigned int localVAbits=8*sizeof(uint64_t*);
  if (localVAbits!=64){
    fprintf(stderr,"Running non-portable code on unsupported platform, terminating. Please use designated machines.\n");
    exit(-1);
  }

  // YOUR CODE GOES HERE
  double numofwords = blocksize / 8;
  double numofsets = (size / blocksize)/assoc;
  acache->numways = assoc;
  acache->blocksize = blocksize;
  acache->numsets = size / blocksize;
  acache->numBitsForBlockOffset = lg2pow2(numofwords);
  acache->numBitsForIndex = lg2pow2(numofsets);
  acache->VAImask = bitmask(acache->numBitsForIndex);
  int numBitsForTag = 61 - acache->numBitsForBlockOffset - acache->numBitsForIndex;
  acache->VATmask = bitmask(numBitsForTag);
 // printf("%u %u %u %u %u %llx %llx\n", acache->numways, acache->blocksize, acache->numsets, acache->numBitsForBlockOffset, acache->numBitsForIndex, acache->VAImask, acache->VATmask);
}


unsigned long long getindex(cache* acache, unsigned long long address){
  unsigned long long temp_addr = address;
  int num_of_shift = acache->numBitsForBlockOffset + 3;
  temp_addr = temp_addr >> num_of_shift;
  return temp_addr & acache->VAImask;
}

unsigned long long gettag(cache* acache, unsigned long long address){
  unsigned long long temp_addr = address;
  int num_of_shift = acache->numBitsForBlockOffset + acache->numBitsForIndex + 3;
  temp_addr = temp_addr >> num_of_shift;
  return temp_addr & acache->VATmask;
}

void writeback(cache* acache, unsigned int index, unsigned int oldestway){
  unsigned long long tag = acache->sets[index].blocks[oldestway].tag;
  unsigned long long addr = (uint64_t) index << (3 + acache->numBitsForBlockOffset) | tag << (3 + acache->numBitsForBlockOffset + acache->numBitsForIndex);
  //using loop to store word by word to the corresponding blockword
  for (int i = 0; i < pow(2,acache->numBitsForBlockOffset); i++){
    StoreWord(acache->nextcache, addr, acache->sets[index].blocks[oldestway].datawords[i]);
    addr += 8;
  }
}

void fill(cache * acache, unsigned int index, unsigned int oldestway, unsigned long long address){
  unsigned long long temp_addr = address;
  unsigned long long addr = (temp_addr >> (3 + acache->numBitsForBlockOffset)) << (3 + acache->numBitsForBlockOffset);
  //looping to read word by word from next level cahce/mem to the correspondign place in upper-level cahce
  for (int i = 0; i < pow(2,acache->numBitsForBlockOffset); i++){
    acache->sets[index].blocks[oldestway].datawords[i] = LoadWord(acache->nextcache, addr);
    addr += 8;
  }
}

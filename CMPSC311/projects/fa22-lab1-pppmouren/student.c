#include "student.h"
#include "string.h"
//squareof smallest
int squareOfSmallest(int array[], int length) {
  //use findMin func to find the samllest value in array// 
  return findMin(&array[0], length) * findMin(&array[0], length);
}



int findMin(int *nums, int numsSize){ 
  int *small = &nums[0];
  
  //use for loo to let *p go through the value in array and compare to *small//
  for (int *p = &nums[0]; p < &nums[numsSize]; ++p){    
    //if *p is samller than the value in *small, assign it to *samll//
    if (*p <= *small){
      *small = *p;
    }
  }
  return *small;
}



bool isPalindrome(char str[]){
  //using strlen() to have the size//
  int length = strlen(str);
  //compare corresponding place to tell if it is palidrome//
  for(char *i = &str[0], *j = &str[length - 1]; i < j; i++, j--){
    if(*i != *j){
      return false;
    }
  }
  
  return true; 
}



int freqOfChar(char str[], char key){
  int count = 0;
  //'\0' is the terminal of the string//
  //loop through the array to find the numbers of key//
  for(char *i = str; *i != '\0'; i++){
    if(*i == key){
      count += 1;
    }
  }
  
  return count;
}



void sort(int array[],int length){
  //use bubble-sort algorithm//
  for(int i = 0; i < length; ++i){
    for(int k = length - 1, j = length - 2; j >= 0; --k, --j){
      //swap two value//
      if (array[j] > array[k]){
	int temp = array[j];
	array[j] = array[k];
	array[k] = temp;
      }
    }
  }
}



int* twoSum(int* nums, int numsSize, int target){
  //nest for loop to add every two value in array//
  for(int *p = nums; p < nums + numsSize - 1; ++p){
    for(int *i = nums + 1; i < nums + numsSize; ++i){
      if (target == (*i + *p)){
	//create a static array for return array of two numbers//
	static int ans[2];
	ans[0] = *p;
	ans[1] = *i;
	return ans;
      }
    }
  }
  return NULL;
}



int* decryptPointer(int array[], int length, int *key[]){
  int *new_arr;
  //malloc() function to create dynamic memory allocation for new_arr//
  new_arr = (int*) malloc(length * sizeof(int));
  
  //loop through both arraies and add up the value to new_arr//
  for(int i = 0;  i < length; i++){
    new_arr[i] = array[i] + *key[i];
    }
  return new_arr;
}





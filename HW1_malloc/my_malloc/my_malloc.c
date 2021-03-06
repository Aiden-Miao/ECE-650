#include "my_malloc.h"

#include <limits.h>

#include "unistd.h"

typedef struct block {
  size_t size;
  struct block * fnext;
} block;

block * fhead = NULL;
//block * ftail = NULL;
int isempty = 1;
size_t data_segment_size = 0;

void rmv_freelist(block * ptr) {
  if (ptr == fhead) {
    if (ptr->fnext == NULL) {
      fhead = NULL;
      return;
    }
    else {
      fhead = ptr->fnext;
      return;
    }
  }
  else {
    block * curr = fhead;
    while (curr->fnext != ptr) {
      curr = curr->fnext;
    }
    curr->fnext = ptr->fnext;
    return;
  }
}

void add_freelist(block * ptr) {
  //  printf("free block size is :%ld\n", ptr->size);
  block * curr = fhead;
  if (fhead == NULL) {
    fhead = ptr;
    fhead->fnext = NULL;
    return;
  }
  if (ptr < fhead) {
    ptr->fnext = fhead;
    fhead = ptr;
    return;
  }
  while (curr->fnext != NULL) {
    if ((curr < ptr) && (ptr < curr->fnext)) {
      block * temp = curr->fnext;
      curr->fnext = ptr;
      ptr->fnext = temp;
      return;
    }
    curr = curr->fnext;
  }
  curr->fnext = ptr;
  ptr->fnext = NULL;
  return;
}

void split(block * ptr, size_t size) {
  rmv_freelist(ptr);
  block * temp;
  /*
  temp = (void *)((char *)ptr + ptr->size - size);
  temp->size = size;
  printf("test1\n");
  ptr->size = ptr->size - size - sizeof(block);
  */
  temp = (void *)((char *)ptr + size + sizeof(block));
  temp->size = ptr->size - size - sizeof(block);
  ptr->size = size;
  //change the data size
  add_freelist(temp);
}
void * find_best_fit(size_t size) {
  block * curr = fhead;
  int diff = INT_MAX;  //set diff as max
  block * ans = NULL;
  int find_exact = 0;
  while (curr != NULL) {
    if ((curr->size >= size)) {  //&& curr->occupied == 0) {
      int temp = (int)(curr->size - size);
      if (temp == 0) {  //if we find a perfect block, stop searching
        ans = curr;
        find_exact = 1;
        break;
        //increase block size
      }
      if (temp < diff) {
        diff = temp;
        ans = curr;
      }
    }
    curr = curr->fnext;
  }
  if (ans != NULL) {
    if (find_exact != 1) {  //if already found a perfect match, return the ans
      if ((int)(ans->size - size - sizeof(block)) < 0) {  //decide if we need to split
                                                          //increase the block size
        rmv_freelist(ans);
        return (void *)((char *)ans + sizeof(block));
      }
      else {
        split(ans, size);
        //return (void *)((char *)ans + sizeof(block) - size + ans->size);
        return (void *)((char *)ans + sizeof(block));
      }
    }
    rmv_freelist(ans);
    return (void *)((char *)ans + sizeof(block));
  }
  else {
    return NULL;
  }
}
void * ff_malloc(size_t size) {
  //list is empty
  if (isempty) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    //printf("size of temp is: %ld\n", temp->size);
    data_segment_size = data_segment_size + size + sizeof(block);  //add size
    //    printf("size grow to: %ld\n", data_segment_size);
    isempty = 0;
    return (void *)((char *)temp + sizeof(block));
  }
  block * curr = fhead;
  //we find a space in the freelist
  while (curr != NULL) {
    if (curr->size >= size) {
      // printf("find a space!\n");
      //printf("curr size is %ld\n", curr->size);
      //printf("size is %ld\n", size);

      if (curr->size > size + sizeof(block)) {
        split(curr, size);
        //choose the latter half as the split area
        //return (void *)((char *)curr + sizeof(block) + curr->size - size);
        return (void *)((char *)curr + sizeof(block));
      }
      rmv_freelist(curr);
      return (void *)((char *)curr + sizeof(block));
    }
    curr = curr->fnext;
  }
  //no space fit in freelist
  block * temp = sbrk(size + sizeof(block));
  temp->size = size;
  data_segment_size = data_segment_size + size + sizeof(block);
  //  printf("size grow to: %ld\n", data_segment_size);
  return (void *)((char *)temp + sizeof(block));
}

void * bf_malloc(size_t size) {
  //list is empty
  if (isempty) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    //printf("size of temp is: %ld\n", temp->size);
    data_segment_size = data_segment_size + size + sizeof(block);  //add size
    //    printf("size grow to: %ld\n", data_segment_size);
    isempty = 0;
    return (void *)((char *)temp + sizeof(block));
  }
  void * ans = find_best_fit(size);
  if (ans != NULL) {
    return ans;
  }
  //no space fit in freelist
  block * temp = sbrk(size + sizeof(block));
  temp->size = size;
  data_segment_size = data_segment_size + size + sizeof(block);
  //  printf("size grow to: %ld\n", data_segment_size);
  return (void *)((char *)temp + sizeof(block));
}

void ff_free(void * ptr) {
  block * temp = (void *)((char *)ptr - sizeof(block));
  add_freelist(temp);
  block * curr = fhead;
  while (curr != temp->fnext) {
    if ((void *)((char *)curr + curr->size + sizeof(block)) == (void *)(curr->fnext)) {
      curr->size = curr->size + curr->fnext->size + sizeof(block);
      curr->fnext = curr->fnext->fnext;
      if (curr->fnext == NULL) {
        break;
      }
      if ((void *)((char *)curr + curr->size + sizeof(block)) == (void *)(curr->fnext)) {
        curr->size = curr->size + curr->fnext->size + sizeof(block);
        curr->fnext = curr->fnext->fnext;
        break;
      }
      break;
    }
    curr = curr->fnext;
  }
}
void bf_free(void * ptr) {
  block * temp = (void *)((char *)ptr - sizeof(block));
  add_freelist(temp);
  block * curr = fhead;
  while (curr != temp->fnext) {
    if ((void *)((char *)curr + curr->size + sizeof(block)) == (void *)(curr->fnext)) {
      curr->size = curr->size + curr->fnext->size + sizeof(block);
      curr->fnext = curr->fnext->fnext;
      if (curr->fnext == NULL) {
        break;
      }
      if ((void *)((char *)curr + curr->size + sizeof(block)) == (void *)(curr->fnext)) {
        curr->size = curr->size + curr->fnext->size + sizeof(block);
        curr->fnext = curr->fnext->fnext;
        break;
      }
      break;
    }
    curr = curr->fnext;
  }
}

unsigned long get_data_segment_size() {
  unsigned long ans = 0;
  block * curr = fhead;
  while (curr != NULL) {
    ans += curr->size;
    curr = curr->fnext;
  }
  ans = data_segment_size - ans;
  return data_segment_size;
}

/*
Calculate the size of free space, we also need to consider the size of metadata.
*/
unsigned long get_data_segment_free_space_size() {
  unsigned long result = 0;
  block * curr = fhead;
  while (curr != NULL) {
    result = result + curr->size + sizeof(block);
    curr = curr->fnext;
  }
  return result;
}

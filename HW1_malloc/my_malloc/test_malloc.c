#include <limits.h>

#include "my_malloc.h"
#include "unistd.h"

typedef struct block {
  size_t size;
  struct block * fnext;
} block;

block * fhead = NULL;
//block * ftail = NULL;
int isempty = 1;

void rmv_freelist(block * ptr) {
  if (ptr == fhead) {
    if (ptr->fnext == NULL) {
      fhead = NULL;
      return;
    }
    else {
      fhead = ptr->fnext;
      ptr->fnext = NULL;
      return;
    }
  }
  else if (ptr->fnext == NULL) {
    block * curr = fhead;
    while (curr->fnext != ptr) {
      curr = curr->fnext;
    }
    curr->fnext = NULL;
    return;
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
  block * curr = fhead;
  if (fhead == NULL) {
    //if the freelist is empty
    fhead = ptr;
    fhead->size = ptr->size;
    return;
  }
  if (ptr < fhead) {
    ptr->fnext = fhead;
    fhead = ptr;
    if ((void *)((char *)ptr + sizeof(block) + ptr->size) == (void *)ptr->fnext) {
      ptr->size = ptr->size + sizeof(block) + ptr->fnext->size;
      block * temp = ptr->fnext;
      ptr->fnext = ptr->fnext->fnext;
      temp->fnext = NULL;
    }
    return;
  }
  while (curr->fnext != NULL) {
    if (curr < ptr && ptr < curr->fnext) {
      if (((void *)((char *)curr + sizeof(block) + curr->size) == (void *)ptr) &&
          ((void *)((char *)ptr + sizeof(block) + ptr->size) == (void *)curr->fnext)) {
        curr->size = curr->size + 2 * sizeof(block) + ptr->size + curr->fnext->size;
        block * temp = curr->fnext;
        curr->fnext = curr->fnext->fnext;
        temp->fnext = NULL;  //remove from freelist
        return;
      }
      if ((void *)((char *)curr + sizeof(block) + curr->size) == (void *)ptr) {
        curr->size = curr->size + sizeof(block) + ptr->size;
        return;
      }
      if ((void *)((char *)ptr + sizeof(block) + ptr->size) == (void *)curr->fnext) {
        block * temp = curr->fnext;
        curr->fnext = ptr;
        ptr->size = ptr->size + sizeof(block) + temp->size;
        ptr->fnext = temp->fnext;
        temp->fnext = NULL;  //remove temp from freelist
        return;
      }
      block * temp = curr->fnext;
      curr->fnext = ptr;
      ptr->fnext = temp;
      return;
    }
    printf("test\n");
    curr = curr->fnext;
  }
  if ((void *)((char *)curr + sizeof(block) + curr->size) == (void *)ptr) {
    curr->size = curr->size + sizeof(block) + ptr->size;
    return;
  }
  curr->fnext = ptr;
  ptr->fnext = NULL;
  return;
}

void split(block * ptr, size_t size) {
  rmv_freelist(ptr);
  block * temp;
  temp = (void *)((char *)ptr + sizeof(block) + size);
  temp->size = ptr->size - size - sizeof(block);
  ptr->size = size;
  add_freelist(temp);
}

void * ff_malloc(size_t size) {
  if (isempty) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    isempty = 0;
    return (void *)((char *)temp + sizeof(block));
  }
  block * curr = fhead;
  while (curr != NULL) {
    if (curr->size >= size) {
      if (curr->size > size + sizeof(block)) {
        split(curr, size);
        return (void *)((char *)curr + sizeof(block));
      }
      rmv_freelist(curr);
      return (void *)((char *)curr + sizeof(block));
    }
    curr = curr->fnext;
  }
  block * temp = sbrk(size + sizeof(block));
  temp->size = size;
  return (void *)((char *)temp + sizeof(block));
}

void ff_free(void * ptr) {
  add_freelist(ptr);
}

unsigned long get_data_segment_size() {
  unsigned long ans = 0;
  block * curr = fhead;
  while (curr != NULL) {
    ans += curr->size + sizeof(block);
    curr = curr->fnext;
  }
  return ans;
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

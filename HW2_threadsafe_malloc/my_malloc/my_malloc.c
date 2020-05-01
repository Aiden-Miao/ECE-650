#include "my_malloc.h"

#include <limits.h>
#include <pthread.h>

#include "unistd.h"

/*
We use one singly-linked freelist here.
*/
typedef struct block {
  size_t size;
  struct block * fnext;
} block;

__thread block * tls_fhead;
block * fhead = NULL;
__thread int tls_isempty = 1;
int isempty = 1;

//we use a global variable to keep track of the whole data size
size_t data_segment_size = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void rmv_freelist(block * ptr) {
  //if it's head, update head.
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
    //remove block in the middle
    block * curr = fhead;
    while (curr->fnext != ptr) {
      curr = curr->fnext;
    }
    curr->fnext = ptr->fnext;
    return;
  }
}

void tls_rmv_freelist(block * ptr) {
  if (ptr == tls_fhead) {
    if (ptr->fnext == NULL) {
      tls_fhead = NULL;
      return;
    }
    else {
      tls_fhead = ptr->fnext;
      return;
    }
  }
  else {
    block * curr = tls_fhead;
    while (curr->fnext != ptr) {
      curr = curr->fnext;
    }
    curr->fnext = ptr->fnext;
    return;
  }
}

void add_freelist(block * ptr) {
  block * curr = fhead;
  //if it's empty, update head.
  if (fhead == NULL) {
    fhead = ptr;
    fhead->fnext = NULL;
    return;
  }
  //add before free head
  if (ptr < fhead) {
    ptr->fnext = fhead;
    fhead = ptr;
    return;
  }
  //add in the middle
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

void tls_add_freelist(block * ptr) {
  block * curr = tls_fhead;
  if (tls_fhead == NULL) {
    tls_fhead = ptr;
    tls_fhead->fnext = NULL;
    return;
  }
  if (ptr < tls_fhead) {
    ptr->fnext = tls_fhead;
    tls_fhead = ptr;
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
  //remove current block from freelist
  rmv_freelist(ptr);
  block * temp;
  temp = (void *)((char *)ptr + size + sizeof(block));
  temp->size = ptr->size - size - sizeof(block);
  //change the data size
  ptr->size = size;
  //add the block left to freelist
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
        rmv_freelist(ans);
        return (void *)((char *)ans + sizeof(block));
      }
      else {
        split(ans, size);
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

void tls_split(block * ptr, size_t size) {
  tls_rmv_freelist(ptr);
  block * temp;
  temp = (void *)((char *)ptr + size + sizeof(block));
  temp->size = ptr->size - size - sizeof(block);
  ptr->size = size;
  tls_add_freelist(temp);
}

void * tls_find_best_fit(size_t size) {
  block * curr = tls_fhead;
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
        tls_rmv_freelist(ans);
        return (void *)((char *)ans + sizeof(block));
      }
      else {
        tls_split(ans, size);
        return (void *)((char *)ans + sizeof(block));
      }
    }
    tls_rmv_freelist(ans);
    return (void *)((char *)ans + sizeof(block));
  }
  else {
    return NULL;
  }
}

void * bf_malloc(size_t size) {
  //list is empty
  if (isempty) {
    block * temp = sbrk(size + sizeof(block));
    temp->size = size;
    data_segment_size = data_segment_size + size + sizeof(block);  //add size
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
  return (void *)((char *)temp + sizeof(block));
}

void bf_free(void * ptr) {
  //get current block and add to freelist
  block * temp = (void *)((char *)ptr - sizeof(block));
  add_freelist(temp);
  block * curr = fhead;
  //merge
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

/*
In LOCK version, we just add lock around free and malloc 
*/
void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void * ans = bf_malloc(size);
  pthread_mutex_unlock(&lock);
  return ans;
}

void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr);
  pthread_mutex_unlock(&lock);
}

/*
NO_LOCK version is almost the same, except we use tls variable
*/
void * ts_malloc_nolock(size_t size) {
  //list is empty
  if (tls_isempty) {
    pthread_mutex_lock(&lock);
    block * temp = sbrk(size + sizeof(block));
    pthread_mutex_unlock(&lock);
    temp->size = size;
    data_segment_size = data_segment_size + size + sizeof(block);  //add size
    tls_isempty = 0;
    return (void *)((char *)temp + sizeof(block));
  }
  void * ans = tls_find_best_fit(size);
  if (ans != NULL) {
    return ans;
  }
  //no space fit in freelist
  pthread_mutex_lock(&lock);
  block * temp = sbrk(size + sizeof(block));
  pthread_mutex_unlock(&lock);
  temp->size = size;
  data_segment_size = data_segment_size + size + sizeof(block);
  return (void *)((char *)temp + sizeof(block));
}

void ts_free_nolock(void * ptr) {
  block * temp = (void *)((char *)ptr - sizeof(block));
  tls_add_freelist(temp);
  block * curr = tls_fhead;
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

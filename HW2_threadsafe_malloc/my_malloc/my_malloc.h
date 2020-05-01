#include <stdio.h>
#include <stdlib.h>

//void * ff_malloc(size_t size);

//void ff_free(void * ptr);

void * bf_malloc(size_t size);

void bf_free(void * ptr);

unsigned long get_data_segment_size();  //in bytes

unsigned long get_data_segment_free_space_size();  //in bytes

void * ts_malloc_lock(size_t size);

void ts_free_lock(void * ptr);

void * ts_malloc_nolock(size_t size);

void ts_free_nolock(void * ptr);

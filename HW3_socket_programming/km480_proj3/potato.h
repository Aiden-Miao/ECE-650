#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

class potato {
 public:
  int hops;
  int hops_cnt;
  int trace[512];
  int total;  //record the total hops
  //int endgame;  //use to determine if the potato we get from ringmaster is already 0
  potato() : hops(0), hops_cnt(0), total(0) { memset(trace, 0, sizeof(trace)); }
  void print_trace() {
    printf("Trace of potato:\n");
    for (int i = 0; i < total; i++) {
      printf("%d", trace[i]);
      if (i == hops_cnt - 1) {
        printf("\n");
      }
      else {
        printf(",");
      }
    }
  }
};

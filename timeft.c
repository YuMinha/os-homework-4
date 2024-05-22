#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "wrapper.h"

typedef struct timelog_tt {
  int count;
  double start_t;
  double sum;
} timelog_t;

timelog_t *timelog;
int timelog_size;
int no_overlap;
int max_overlap;
int max_count;
double init_t;
Semaphore time_sem;

void init_timelog(int size)
{
  int i;
  struct timeval tv;
  
  timelog = (timelog_t *)malloc(size * sizeof(timelog_t));
  timelog_size = size;
  for (i = 0; i < size; i++){
    timelog[i].start_t = 0.0;
    timelog[i].sum = 0.0;
  }
  no_overlap = 0;
  max_overlap = 0;
  max_count = 0;
  gettimeofday(&tv, NULL);
  init_t = tv.tv_sec + 0.000001 * tv.tv_usec;
  Sem_init(&time_sem, 1);
}

/* save start time in timelog[idx] */
void start_timelog(int idx)
{
  struct timeval tv;

  if (idx > timelog_size){
    fprintf(stderr, "index of timelog(%d) should be smaller than thread size(%d).\n", idx, timelog_size);
    exit(1);
  }
  gettimeofday(&tv, NULL);
  timelog[idx].start_t = tv.tv_sec + 0.000001 * tv.tv_usec;
  Sem_wait(&time_sem);
  no_overlap++;
  if (no_overlap > max_overlap) {
    max_overlap = no_overlap;
    max_count = 0;
  } else if (no_overlap == max_overlap)
    max_count++;
  Sem_post(&time_sem);
}

void finish_timelog(int idx)
{
  struct timeval tv;
  double elapsed_t;
  
  if (idx > timelog_size){
    fprintf(stderr, "index of timelog(%d) should be smaller than thread size(%d).\n", idx, timelog_size);
    exit(1);
  }
  if (timelog[idx].start_t == 0.0){
    fprintf(stderr, "finishe_timelog() should be called before start_timelog().\n");
    exit(1);
  }
  gettimeofday(&tv, NULL);
  elapsed_t = tv.tv_sec + 0.000001 * tv.tv_usec - timelog[idx].start_t;
  if (elapsed_t < 0.0){
    fprintf(stderr, "finishe_timelog() is called before start_timelog().\n");
    exit(1);
  }
  timelog[idx].sum += elapsed_t;
  Sem_wait(&time_sem);
  no_overlap--;
  Sem_post(&time_sem);
}

void close_timelog(void)
{
  double total_t;
  double scale_up;
  double sum;
  struct timeval tv;
  int i;
  
  gettimeofday(&tv, NULL);
  total_t = tv.tv_sec + 0.000001 * tv.tv_usec - init_t;
  if (total_t == 0.0){
    fprintf(stderr, "Running time is too short.\n");
    exit(1);
  }
  sum = 0.0;
  for (i = 0; i < timelog_size; i++)
    sum += timelog[i].sum;
  scale_up = sum / total_t;
  printf("%d %d %lf\n", max_overlap, max_count, scale_up);
  free(timelog);
  Sem_destroy(&time_sem);
}

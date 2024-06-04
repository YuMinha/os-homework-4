#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "wrapper.h"

void init_timelog(int size);
void start_timelog(int idx);
void finish_timelog(int idx);
void close_timelog(void);


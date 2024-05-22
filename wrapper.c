#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "wrapper.h"

int Open(const char *path, int oflag)
{
  int fd = open(path, oflag);

  if (fd < 0){
    fprintf(stderr, "file(%s) open error:%s\n", path, strerror(errno));
    exit(1);
  }
  return(fd);
}

int Creat(const char *path, mode_t mode)
{
  int fd = creat(path, mode);

  if (fd < 0){
    fprintf(stderr, "file(%s) creat error:%s\n", path, strerror(errno));
    exit(1);
  }
  return(fd);
}

int Read(int filedes, void *buf, size_t nbyte)
{
  int res = read(filedes, buf, nbyte);

  if (res < 0){
    if (errno == EINTR)
      fprintf(stderr, "read is stopped by signal.\n");
    else
      fprintf(stderr, "read error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Write(int filedes, const void *buf, size_t nbyte)
{
  int res = write(filedes, buf, nbyte);

  if (res < 0){
    if (errno == EINTR)
      fprintf(stderr, "write is stopped by signal.\n");
    else
      fprintf(stderr, "write error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

void *Mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  void *res = mmap(addr, length, prot, flags, fd, offset);

  if (res < 0){
    fprintf(stderr, "mmap error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Stat(const char *restrict path, struct stat *restrict buf)
{
  int res = stat(path, buf);

  if (res < 0){
    fprintf(stderr, "stat error on %s: %s.\n", path, strerror(errno));
    exit(1);
  }
  return(res);
}

int Fstat(int fildes, struct stat *buf)
{
  int res = fstat(fildes, buf);

  if (res < 0){
    fprintf(stderr, "fstat error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Ftruncate(int fd, off_t length)
{
  int res = ftruncate(fd, length);

  if (res < 0){
    fprintf(stderr, "ftruncate error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count)
{
  int res = pthread_barrier_init(barrier, attr, count);

  if (res < 0){
    fprintf(stderr, "pthread_barrier_init error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
  int res = pthread_barrierattr_init(attr);

  if (res < 0){
    fprintf(stderr, "pthread_barrierattr_init error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
  int res = pthread_barrierattr_setpshared(attr, pshared);

  if (res < 0){
    fprintf(stderr, "pthread_barrierattr_setpshared error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

int Pthread_create(pthread_t *thread, const pthread_attr_t * attr, void *(*start_routine)(void *), void *arg)
{
  int res = pthread_create(thread, attr, start_routine, arg);

  if (res < 0){
    fprintf(stderr, "pthread_create error: %s.\n", strerror(errno));
    exit(1);
  }
  return(res);
}

/*
  The followings are counting semaphore implementation that has minus value
*/
void Sem_init(Semaphore *Sem, int init_value)
{
  Sem->value = init_value;
  pthread_cond_init(&Sem->cond, NULL);
  pthread_mutex_init(&Sem->lock, NULL);
}

void Sem_wait(Semaphore *Sem)
{
  pthread_mutex_lock(&Sem->lock);
  while (Sem->value <= 0)
    pthread_cond_wait(&Sem->cond, &Sem->lock);
  Sem->value--;
  pthread_mutex_unlock(&Sem->lock);
}

void Sem_post(Semaphore *Sem)
{
  pthread_mutex_lock(&Sem->lock);
  Sem->value++;
  pthread_cond_signal(&Sem->cond);
  pthread_mutex_unlock(&Sem->lock);
}

void Sem_destroy(Semaphore *Sem)
{
  pthread_cond_destroy(&Sem->cond);
  pthread_mutex_destroy(&Sem->lock);
}


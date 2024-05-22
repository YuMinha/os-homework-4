/* 실수 연산이 반복될 때 발생하는 round off 에러를 고려해서 두 binary file의 내용을 비교하는 코드 */ 
/* 두 파일이 n*n개의 float 실수 (행렬)들로 이루어져 있고, 두 파일의 각 실수들의 차이가 세 번째 argument보다 크면 error를 출력한다. 두 파일의 크기는 n*n*sizeof(float) byte이어야 한다. 아래 예서는 c1.dat 파일과 c2.dat 파일을 비교할 때 각 float element의 차이가 0.001 미만일 때는 동일한 것으로 판정하여 0을 returns하고 그렇지 않으면 1을 return한다.
   compile: gcc -g -Wall -o hwdiff hwdiff.c -lm
   실행예: ./hwdiff c1.dat c2.dat 0.001   
 */
/* 1을 return하는 오류 상황:
   - 첫 번째 파일을 열 수 없거나 n*n float 크기가 아닐 때
   - 두 번째 파일을 열 수 없거나 첫 번째 파일과 같은 크기가 아닐 때
   - 값이 다른 element가 발생할 때 
 */
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include "wrapper.h"

static inline int issquare(int val)
{
  int res =  (int)round(sqrt(val));
  return (res * res == val);
}

static inline size_t noElement(int fd)
{
  struct stat statbuf;
  
  Fstat(fd, &statbuf);
  return(statbuf.st_size/sizeof(double));
}

double **alloc_matrix(int k)
{
  double **mptr;
  double *tmp; /* used for allocating a big memory */
  int i;

  tmp = (double *)malloc(k * k * sizeof(double));
  assert(tmp);
  mptr = (double **)malloc(k * sizeof(double *));
  for (i = 0; i < k; i++)
    mptr[i] = tmp + i * k;
  return(mptr);
}

void free_matrix(double **mptr)
{
  double *tmp = mptr[0];

  free(mptr);
  free(tmp);
}

int main(int argc, char *argv[])
{
  int i;
  double diff;
  int afd, bfd; /* file descriptor */
  size_t afsize, bfsize;
  double *aaddr, *baddr;
  
  if (argc != 4){
    fprintf(stderr, "Usage: %s <file1> <file2> <diff>\n", argv[0]);
    exit(1);
  }
  
  afd = Open(argv[1], O_RDONLY);
  afsize = noElement(afd);
  bfd = Open(argv[2], O_RDONLY);
  bfsize = noElement(bfd);
  if (bfsize != afsize){
    fprintf(stderr, "The 2nd file(%s)'s size is not the same as the 1st's one.\n", argv[2]);
    exit(1);
  }
  aaddr = Mmap(0, sizeof(double) * afsize, PROT_READ, MAP_PRIVATE, afd,0);
  baddr = Mmap(0, sizeof(double) * bfsize, PROT_READ, MAP_PRIVATE, bfd,0);
  diff = atof(argv[3]);
  for (i = 0; i < afsize; i++){
    if (fabs(aaddr[i] - baddr[i]) > diff){
      printf("Values %lf and %lf at location %d are different more than %lf.\n", aaddr[i], baddr[i], i, diff);
      munmap(0, sizeof(double) * afsize);
      munmap(0, sizeof(double) * bfsize);
      close(afd);
      close(bfd);
      exit(1);
    }
  }
  munmap(aaddr, sizeof(double) * afsize);
  munmap(baddr, sizeof(double) * bfsize);
  close(afd);
  close(bfd);
  exit(0);
}

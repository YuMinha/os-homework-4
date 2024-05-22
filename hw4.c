/* Description:
     It finds a vector x of a equation Ax = b
     where n x n matrix A and vector b of size n are given
     using a thread pool method.
   Usage:
     ./hw4 A.dat b.dat x.dat <n>
     A.dat and b.dat are inputs, x.dat is the output.
     <n> is the thread pool size 
   Note:
     Current 'insufficient' code is restricted to 2 x 2 matrix and 2 sized vector and single-process. Complete the code to cover general n x n matrix and n sized vector and solve the equation using Gaussian elimination and thread pool
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "wrapper.h"
#include "timeft.h"

double A[4], B[2], X[2];

void argCheck(int argc, char *argv[])
{
  /*
    Fill the function with codes such that
    if the number of arguments is not 4
       exit with code 1 and the following error message
           Usage: hw1 <mat> <invec> <outvec>
   */
}

int sizeCheck(int argc, char *argv[])
{
  /*
     Fill the function with codes such that 
     if the number of elements in file argv[1] is the square of the argv[2]
        return the number of elements in file argv[2]
     else
        exit with value 1
  */
  return(0);
}

void getData(int size, int argc, char *argv[])
{
  int afd, bfd;
  
  afd = Open(argv[1], O_RDONLY);
  bfd = Open(argv[2], O_RDONLY);
  Read(afd, A, sizeof(double) * size * size);
  Read(bfd, B, sizeof(double) * size);
  close(afd);
  close(bfd);
}

int main(int argc, char *argv[])
{
  int xfd;
  double dete; /* 행렬 A의 determinant */
  int size;

  argCheck(argc, argv);
  size = sizeCheck(argc, argv);
  getData(size, argc, argv);
  init_timelog(1); // 크기 1의 thread pool을 사용한다.
  
  /* 여기서 연립방정식을 풀어서 X[0]과 X[1]을 구한다. */
  start_timelog(0); // 0번 thread의 한 작업구간 기록 시작 
  dete = 1; /* 수정 필요 */
  X[0] = dete;  /* 수정 필요 */
  X[1] = dete;  /* 수정 필요 */
  finish_timelog(0); // 0번 thread의 한 작업구간 기록 끝
  xfd = Creat(argv[3], 0644);
  Write(xfd, X, sizeof(double) * 2);
  close(xfd);
  close_timelog();
  return 0;
}

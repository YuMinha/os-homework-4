/* double type의 이진 파일을 보여준다.  
   Usage: ./showdata <data> 
     <data>를 vector의 형태로 보여준다. 
   Usage: ./showdata m <data>
     <data>를 matrix의 형태로 보여준다.
*/
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "wrapper.h"

#define FNLEN 512

int main(int argc, char *argv[])
{
  struct stat finfo;
  int noele;
  int j;
  double *Ar;
  int type;
  int fd;
  
  /* check no. arguments */
  if (argc != 2 && argc != 3){
    fprintf(stderr, "Usage:\t%s <data>\n\t%s m <data>\n", argv[0], argv[0]);
    exit(1);
  }

  /* check no. elements in files */
  if (argc == 2){
    type = 0; // vector
    Stat(argv[1], &finfo);
    fd = Open(argv[1], O_RDONLY);
  } else { // matrix
    if (strncmp(argv[1], "m", 1) != 0){
      fprintf(stderr, "Usage: %s\tm <data>\n", argv[0]);
      exit(1);
    }
    type = 1;
    Stat(argv[2], &finfo);
    fd = Open(argv[2], O_RDONLY);
  }
  noele = finfo.st_size / sizeof(double);
  Ar = (double *)malloc(sizeof(double) * noele);
  assert(Ar);
  Read(fd, Ar, sizeof(double) * noele);
  close(fd);
  if (type == 0){ // vector
    for (j = 0; j < noele; j++)
      printf("%7.2lf ", Ar[j]);
    printf("\n");
  } else { // matrix
    int col = (int)sqrt(noele);
    int cnt = 0;
    do {
      for (j = 0; j < col; j++){
	printf("%7.2lf ", Ar[cnt++]);
	if (cnt >= noele) break;
      }
      printf("\n");
    } while (cnt < noele);
  }
  free(Ar);
}

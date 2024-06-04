#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "wrapper.h"
#include "timeft.h"

#define BUFFER_SIZE 10

typedef enum {
    GAUSSIAN,
    BACK_SUB,
    TERMINATE
} OperationType;

typedef struct {
    int row, col;
    OperationType operation;
} Task;

Task bounded_buffer[BUFFER_SIZE];
int buffer_head = 0, buffer_tail = 0;

Semaphore full, empty, mutex, gaussian_sem, backsub_sem;

double *A, *B, *X;
int size, nt;


void argCheck(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <mat> <invec> <outvec> <n>\n", argv[0]);
        exit(1);
    }
    if (atoi(argv[4]) <= 0) {
        fprintf(stderr, "Error: number of thread is not a positive integer.\n");
        exit(1);
    }
}

int sizeCheck(char *argv[]) {
    struct stat finfoA, finfoB;
    int noeleA, noeleB;

    stat(argv[1], &finfoA);
    stat(argv[2], &finfoB);
    noeleA = finfoA.st_size / sizeof(double);
    noeleB = finfoB.st_size / sizeof(double);
    if (noeleA != noeleB * noeleB) {
        fprintf(stderr, "Error: number of elements in %s is not square of that in %s.\n", argv[1], argv[2]);
        exit(1);
    }
    return noeleB;
}

void getData(int size, char *argv[]) {
    int afd = Open(argv[1], O_RDONLY);
    int bfd = Open(argv[2], O_RDONLY);

    A = (double*)malloc(sizeof(double) * size * size);
    B = (double*)malloc(sizeof(double) * size);
    X = (double*)malloc(sizeof(double) * size);

    Read(afd, A, sizeof(double) * size * size);
    Read(bfd, B, sizeof(double) * size);

    close(afd);
    close(bfd);
}

void enqueue(Task task) {
    Sem_wait(&empty);
    Sem_wait(&mutex);

    bounded_buffer[buffer_tail] = task;
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;

    Sem_post(&mutex);
    Sem_post(&full);
}

Task dequeue() {
    Sem_wait(&full);
    Sem_wait(&mutex);

    Task task = bounded_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;

    Sem_post(&mutex);
    Sem_post(&empty);

    return task;
}

void *worker_thread(void *arg) {
    int idx = *(int *)arg;
    free(arg);

    while (1) {
        Task task = dequeue();
        if (task.operation == TERMINATE) break;

        start_timelog(idx);

        int row = task.row, col = task.col;
        if (task.operation == GAUSSIAN) {
            double pivot = A[col * size + col];
            double factor = A[row * size + col] / pivot;
            for (int j = col; j < size; ++j) {
                A[row * size + j] -= factor * A[col * size + j];
            }
            B[row] -= factor * B[col];
            Sem_post(&gaussian_sem);
        } else if (task.operation == BACK_SUB) {
            double sum = 0;
            for (int j = row + 1; j < size; ++j) {
                sum += A[row * size + j] * X[j];
            }
            X[row] = (B[row] - sum) / A[row * size + row];
            Sem_post(&backsub_sem);
        }

        finish_timelog(idx);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    argCheck(argc, argv);
    size = sizeCheck(argv);
    nt = atoi(argv[4]);

    getData(size, argv);

    pthread_t threads[nt];
    Sem_init(&full, 0);
    Sem_init(&empty, BUFFER_SIZE);
    Sem_init(&mutex, 1);
    Sem_init(&gaussian_sem, 0);
    Sem_init(&backsub_sem, 0);

    init_timelog(nt);

    for (int i = 0; i < nt; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&threads[i], NULL, worker_thread, arg);
    }

    
    for (int col = 0; col < size - 1; col++) {
        for (int row = col + 1; row < size; row++) {
            enqueue((Task){row, col, GAUSSIAN});
        }
        for (int row = col + 1; row < size; row++) {
            Sem_wait(&gaussian_sem);
        }
    }

    for (int row = size - 1; row >= 0; row--) {
        enqueue((Task){row, row, BACK_SUB});
        for (int col = 0; col < row; col++) {
            enqueue((Task){col, row, BACK_SUB});
        }
        for (int col = 0; col <= row; col++) {
            Sem_wait(&backsub_sem);
        }
    }

    for (int i = 0; i < nt; i++) {
        enqueue((Task){-1, -1, TERMINATE});
    }

    for (int i = 0; i < nt; i++) {
        pthread_join(threads[i], NULL);
    }

    int xfd = Creat(argv[3], 0644);
    Write(xfd, X, sizeof(double) * size);
    close(xfd);

    Sem_destroy(&full);
    Sem_destroy(&empty);
    Sem_destroy(&mutex);
    Sem_destroy(&gaussian_sem);
    Sem_destroy(&backsub_sem);

    close_timelog();

    free(A);
    free(B);
    free(X);

    return 0;
}


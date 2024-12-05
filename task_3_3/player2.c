#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

#define THRESHOLD 20

int main() {
    int shmid;
    double *shared_time;
    char pathname[] = "player1.c";
    key_t key = 123;

    shmid = shmget(key, sizeof(double), 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shared_time = (double *)shmat(shmid, NULL, 0);
    if (shared_time == (double *) -1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        printf("Ход черных. Нажмите Enter, чтобы завершить ход...\n");
        getchar();

        double start_time = (double)clock() / _SC_CLK_TCK;
        double elapsed_time = (double)clock() / _SC_CLK_TCK - start_time;
        *shared_time += elapsed_time;

        if (*shared_time > THRESHOLD) {
            printf("Техническое поражение черных\n");
            break;
        }
        printf("Время черных: %.2f секунд\n", *shared_time);
    }
    shmdt(shared_time);
    return 0;
}
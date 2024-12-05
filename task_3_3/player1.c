#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

#define THRESHOLD 20 //Порог времени

int main() {
    int shmid; //IPC дескриптор
    double *shared_time; //Указатель на разделяемую память
    char pathname[] = "player1.c";
    key_t key = 123; //Задал одинаковый ключ для обеих программ
    //Получение/создание сегмента разделяемой памяти
    shmid = shmget(key, sizeof(double), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    //Присоединение к сегменту разделяемой памяти
    shared_time = (double *)shmat(shmid, NULL, 0);
    if (shared_time == (double *) -1) {
        perror("shmat");
        exit(1);
    }

    *shared_time = 0;
    while (1) {
        printf("Ход белых. Нажмите Enter, чтобы завершить ход...\n");
        getchar();

        //Расчет времени хода
        double start_time = (double)clock() / _SC_CLK_TCK;
        double elapsed_time = (double)clock() / _SC_CLK_TCK - start_time;
        *shared_time += elapsed_time; //Обновление общего времени

        if (*shared_time > THRESHOLD) {
            printf("Техническое поражение белых\n");
            break;
        }
        printf("Время белых: %.2f секунд\n", *shared_time);
    }
    shmdt(shared_time);
    return 0;
}
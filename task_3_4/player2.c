#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/chess_shm"
#define SEM_WHITE "/sem_white"
#define SEM_BLACK "/sem_black"
#define SHM_SIZE 100

int main() {
    int shm_fd;
    char *shared_memory;
    sem_t *sem_white, *sem_black;
    char move[SHM_SIZE];

    // Открыть разделяемую память
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    // Отобразить разделяемую память в адресное пространство процесса
    shared_memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // Открыть семафоры
    sem_white = sem_open(SEM_WHITE, 0);
    if (sem_white == SEM_FAILED) {
        perror("sem_open (white)");
        munmap(shared_memory, SHM_SIZE);
        close(shm_fd);
        exit(1);
    }
    sem_black = sem_open(SEM_BLACK, 0);
    if (sem_black == SEM_FAILED) {
        perror("sem_open (black)");
        munmap(shared_memory, SHM_SIZE);
        close(shm_fd);
        exit(1);
    }
    char* black_moves[] = {"c5", "d6", "b5", "a4", "b4", NULL};
    int move_index = 0;

    while (black_moves[move_index] != NULL) {
        sem_wait(sem_black);   // ждем семафор черных
        strcpy(move, black_moves[move_index]);
        printf("Черные: %s\n", move);
        move_index++;
        sem_post(sem_white);  // Разрешаем ход белым
        sleep(1);
    }
    munmap(shared_memory, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_white);
    sem_close(sem_black);

    return 0;
}

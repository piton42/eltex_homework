#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;

    // 1. Вывод PID процесса
    pid_t my_pid = getpid();
    printf("My PID: %d\n", my_pid);

    // 2. Ждет 1 секунду
    sleep(1);


    // Проверяем аргументы командной строки.  Если есть аргумент "--no-fork", не создаем дочерний процесс.
    if (argc > 1 && strcmp(argv[1], "--no-fork") == 0) {
        return 0;
    } else {
        // 3. exec самой себя, но только если нет флага --no-fork
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            // Добавляем "--no-fork" к аргументам для следующего запуска
            char *new_argv[] = {argv[0], "--no-fork"};
            for (int i = 1; i < argc; i++) {
              new_argv[i + 1] = argv[i];
            }
            new_argv[argc + 1] = NULL; // добавляем NULL в конце

            execv(argv[0], new_argv);
            perror("execv failed");
            return 1;
        } else {
            // Родительский процесс дожидается завершения дочернего
            wait(NULL);

            // 4. Вывод сообщения которое поступает на вход (в родительском процессе)
            for (int i = 1; i < argc; i++) {
                printf("%s ", argv[i]);
            }
            printf("\n");
            return 0;
        }
    }
}


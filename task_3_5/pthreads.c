#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

//Глобальные переменные для завершения thread2 и thread3
volatile sig_atomic_t thread2_running = 1;
volatile sig_atomic_t thread3_running = 1;

//Обработчик SIGINT
void sigint_handler(int signum) {
    if (signum == SIGINT) {
        printf("Thread 2: Recieved SIGINT. Finishing work.\n");
        thread2_running = 0;
    }
}

//Функции для 1,2 и 3 потока

void *thread1_func(void *arg) {
sigset_t mask; //создаем маску для блокировки всех сигналов
     sigfillset(&mask); //заполняем
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        perror("pthread_sigmask");
         pthread_exit(NULL);
    }
    printf("Thread 1: All signals are blocked\n");
    int i;
    int loc_id = * (int *) arg;
     for (i = 0; i < 10; i++) {
        printf("Thread %i is running\n", loc_id);
        sleep(1);
    }
    pthread_exit(NULL);
}

void *thread2_func(void *arg) {
    int loc_id = * (int *) arg;
    printf("Thread 2: Starting work. Waiting SIGINT.\n");
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        pthread_exit(NULL); //Завершаем pthread, если SIGINT
    }
    
    //Цикл для того чтобы знать, когда pthread работает
    while (thread2_running) {
        printf("Thread 2 is running\n");
         sleep(1);
    }
        printf("Thread 2: Finishing work.\n");
    pthread_exit(NULL);
}

void *thread3_func(void *arg) {
 int loc_id = *(int *)arg;
     printf("Thread 3: Starting work. Waiting SIGQUIT.\n");

    sigset_t mask;
    sigemptyset(&mask); //очищаем маску
    sigaddset(&mask, SIGQUIT); //добавляем в маску SIGQUIT

    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        perror("pthread_sigmask for thread3");
        pthread_exit(NULL);
    }

    int sig;
    while (thread3_running) {
        if (sigwait(&mask, &sig) != 0) { //sigwait блокирует поток пока не получен сигнал SIGQUIT
            if (errno == EINTR) {
                continue; // Если прервано сигналом, продолжаем цикл
            }
             perror("sigwait");
             thread3_running = 0;
            break;
        }
        if (sig == SIGQUIT) {
            printf("Thread 3: Received SIGQUIT. Finishing work.\n");
            thread3_running = 0;
        }
        printf("Thread 3 is running\n");
        sleep(1);
    }
       printf("Thread 3: Finishing work\n");
    pthread_exit(NULL);
}

int main(int argc, char * argv[]){
    
    pid_t pid;
    pid_t my_pid = getpid();
    printf("My PID: %d\n", my_pid);

  int id1, id2, id3, result;
  pthread_t thread1, thread2, thread3;
  id1 = 1;
  result = pthread_create(&thread1, NULL, thread1_func, &id1);
  if (result != 0) {
    perror("Creating the first thread");
    return EXIT_FAILURE;
  }

  id2 = 2;
  result = pthread_create(&thread2, NULL, thread2_func, &id2);
  if (result != 0) {
    perror("Creating the second thread");
    return EXIT_FAILURE;
  }

  id3 = 3;
  result = pthread_create(&thread3, NULL, thread3_func, &id3);
  if (result != 0) {
    perror("Creating the third thread");
    return EXIT_FAILURE;
  }

  result = pthread_join(thread1, NULL);
  if (result != 0) {
    perror("Joining the first thread");
    return EXIT_FAILURE;
  }

  result = pthread_join(thread2, NULL);
  if (result != 0) {
    perror("Joining the second thread");
    return EXIT_FAILURE;
  }

  result = pthread_join(thread3, NULL);
  if (result != 0) {
    perror("Joining the third thread");
    return EXIT_FAILURE;
  }

  printf("Done\n");
  return EXIT_SUCCESS;
}
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define FIFO_PATH "fifo_fn"
#define MESSAGE "SIGABRT recieved\n"
#define FIFO_MODE 0666

void sigabrt_handler(int sig) {
	printf("SIGABRT handler called\n");
	int fd;
	if ((fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK)) == -1) {
		if (errno != ENXIO) { //Проверяем что FIFO существует
			perror("open");
			exit(1);
		} else {
			fprintf(stderr, "FIFO does not exist\n");
		}
		return; //не завершаем программу если FIFO не существует
	}
	if (write(fd, MESSAGE, strlen(MESSAGE)) == -1) {
		perror("write");
		exit(1);
	}
	close(fd);
}
int main() {

	mode_t mode = O_WRONLY;
 	const char * fifo_fn = "fifo_fn";
 	int fd_fifo;
 	int res = mknod(fifo_fn, 755, 0);
 	if (fd_fifo = open(fifo_fn, mode, O_NDELAY) < 0)
 	{
  		printf("can not open");
  		exit(-1);
 	}

	//Установка обработчика сигнала SIGABRT
	struct sigaction sa;
	sa.sa_handler = sigabrt_handler;   
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGABRT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Программа ожидает сигнала SIGABRT. PID процесса: %d\n", getpid());
	while(1) {
		pause();
	}
	return 0;
}

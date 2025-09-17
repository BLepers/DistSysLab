/*
* Usage ./critical <process ID> <sleep duration>
*
* Output:
* [Process \d+] [Time \d+] Lock taken
* [Process \d+] [Time \d+] Lock released
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include<time.h>

static unsigned long  current_time(void) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static void append(int fd, int pid, int release) {
	 char *msg;
	 int written = 0;
    int len = asprintf(&msg, "[Process %d] [Time %lu] Lock %s\n", pid, current_time(), release ? "released" : "taken");
    assert(len > 0);

	 while(written < len) {
		 int ret = write(fd, msg + written, len - written);
		 if(ret == -1) {
			 perror("Failed to write to log.txt file");
			 free(msg);
			 exit(1);
		 }
		 written += ret;
	 }

	 fsync(fd);
	 free(msg);
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Usage: %s <process ID> <sleep duration>\n", argv[0]);
		return 1;
	}

	int pid = atoi(argv[1]);
	int sleep_duration = atoi(argv[2]);
	int log_fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (log_fd == -1) {
		perror("Failed to open log.txt file");
		return 1;
	}

	append(log_fd, pid, 0);
	sleep(sleep_duration);
	append(log_fd, pid, 1);

	return 0;
}

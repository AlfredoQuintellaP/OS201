#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

#define FILDES_IN 0
#define FILDES_OUT 1
#define FILDES_ERROR 2

int cmd_plus(char *buffer, int out_fileno) {
    int arg1, arg2, n;
    sscanf(buffer, "+%d,%d", &arg1, &arg2);

    n = sprintf(buffer, "%d", arg1 + arg2);
    write(out_fileno, buffer, n);
    return 0;
}

int cmd_minus(char *buffer, int out_fileno) {
    int arg1, arg2, n;
    sscanf(buffer, "-%d,%d", &arg1, &arg2);

    n = sprintf(buffer, "%d", arg1 - arg2);
    write(out_fileno, buffer, n);
    return 0;
}

int cmd_exec(char *buffer) {
    return system(buffer+1);
}

void child_process(int read_fd, int write_fd) {
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while((bytes_read = read(read_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        if(bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }

        switch(buffer[0]) {
            case '+':
                cmd_plus(buffer, write_fd);
                break;

            case '-':
                cmd_minus(buffer, write_fd);
                break;

            case 'e':
                cmd_exec(buffer);
                break;

            default:
                write(write_fd, "Unknown command\n", 16);
                break;
        }

        write(write_fd, "\n", 1);
    }

    syscall(SYS_exit, 0);
    //return;
}

void parent_process(int write_fd, int read_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while((bytes_read = read(0, buffer, BUFFER_SIZE - 1)) > 0) {
        write(write_fd, buffer, bytes_read);

        char result[BUFFER_SIZE];
        ssize_t n = read(read_fd, result, BUFFER_SIZE - 1);
        if(n > 0) {
            result[n] = '\0';
            write(FILDES_OUT, result, n);
        }
    }

    close(write_fd);
    close(read_fd);

    wait(NULL);
}

int main() {
    int pipe1[2];
    int pipe2[2];

    if((pipe(pipe1) == -1) || (pipe(pipe2) == -1)) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();

    if(pid == -1) {
        perror("fork");
        exit(1);
    }

    if(pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        
        child_process(pipe1[0], pipe2[1]);

        close(pipe1[0]);
        close(pipe2[1]);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);

        parent_process(pipe1[1], pipe2[0]);

        close(pipe1[1]);
        close(pipe2[0]);
    }

    return 0;
}

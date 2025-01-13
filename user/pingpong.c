#include "kernel/types.h"
#include "user/user.h"

int main() {
    // pipe[0]: read, pipe[1]: write
    int pipe1[2]; // parent to child
    int pipe2[2]; // child to parent
    
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        printf("pipe failed\n");
        exit(1);
    }
    int pid = fork();
    if (pid < 0) {
        printf("fork failed\n");
        exit(1);
    } else if (pid == 0) { // parent to child
        close(pipe1[1]);
        close(pipe2[0]);
        char c;
        if (read(pipe1[0], &c, 1) != 1) {
            printf("pipe1 read failed\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        c = '$';
        if (write(pipe2[1], &c, 1) != 1) {
            printf("pipe2 write failed\n");
            exit(1);
        }
        close(pipe1[0]);
        close(pipe2[1]);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);
        char c = '$';
        if (write(pipe1[1], &c, 1) != 1) {
            printf("pipe2 write failed\n");
            exit(1);
        }
        if (read(pipe2[0], &c, 1) != 1) {
            printf("pipe1 read failed\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());
        close(pipe1[1]);
        close(pipe2[0]);
    }
    wait(0);
    return 0;
}
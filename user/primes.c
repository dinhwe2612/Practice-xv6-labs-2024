#include "kernel/types.h"
#include "user/user.h"

void primes(int p_left) __attribute__((noreturn));

void primes(int p_left) {
    int prime;

    // Read the first number from the pipe
    if (read(p_left, &prime, sizeof(prime)) == 0) {
        close(p_left);
        exit(0); // No more numbers, exit
    }

    // Print the prime number
    printf("prime %d\n", prime);

    // Create a new pipe for the next process
    int p_right[2];
    pipe(p_right);

    if (fork() == 0) {
        // Child process: recursively process the next stage
        close(p_right[1]);  // Close unused write end
        close(p_left);      // Close inherited read end
        primes(p_right[0]);
    } else {
        // Parent process: filter numbers and pass to the next stage
        close(p_right[0]);  // Close unused read end

        int num;
        while (read(p_left, &num, sizeof(num)) > 0) {
            if (num % prime != 0) {
                write(p_right[1], &num, sizeof(num));
            }
        }

        // Close pipes and wait for the child process
        close(p_left);
        close(p_right[1]);
        wait(0);
        exit(0);
    }
}

int main() {
    int p[2];
    pipe(p);

    if (fork() == 0) {
        // Child process: start the pipeline
        close(p[1]); // Close unused write end
        primes(p[0]);
    } else {
        // Parent process: feed numbers into the pipeline
        close(p[0]); // Close unused read end

        for (int i = 2; i <= 280; i++) {
            write(p[1], &i, sizeof(i));
        }

        // Close the write end of the pipe
        close(p[1]);
        wait(0); // Wait for the child process to finish
        exit(0);
    }
}
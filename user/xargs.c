#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

const int MAX_BUF = 512;

int main(int argc, char *argv[]) {

    int limit = 1;
    if (argc > 2 && strcmp(argv[1], "-n") == 0) {
        limit = atoi(argv[2]);
        if (limit <= 0) {
            fprintf(2, "invalid value for -n\n");
            exit(1);
        }
        argc -= 2;
        argv += 2;
    }
    argc -= 1;
    argv += 1;
    char buf[MAX_BUF];
    char *args[MAXARG];
    for(int i = 0; i < argc; ++i) {
        args[i] = argv[i];
    }
    int arg_count = argc;
    while(1) {
        int buf_count = 0;
        while(buf_count + 1 < MAX_BUF) {
            char c;
            int read_res = read(0, &c, 1);
            if (read_res <= 0) {
                break;
            }
            if (c == '\n') { // if meet '\n' then split into another command
                break;
            }
            buf[buf_count++] = c;
        }
        buf[buf_count] = '\0';
        if (buf_count == 0 && arg_count == argc) {// there is no input
            break;
        }
        if (buf_count > 0) {
            // Allocate memory for the current input line
            args[arg_count] = malloc(buf_count + 1);
            if (args[arg_count] == 0) {
                fprintf(2, "xargs: malloc failed\n");
                exit(1);
            }
            memmove(args[arg_count], buf, buf_count + 1);
            arg_count++;
        }
        if (arg_count == limit + argc || buf_count == 0) {
            args[arg_count] = '\0';
            if (fork() == 0) {
                exec(args[0], args);
                fprintf(2, "exec failed\n");
                exit(1);
            }
            wait(0);
            arg_count = argc;
        }
        if (buf_count == 0) {
            break;
        }
    }
    return 0;
}
//
// Created by 付旭炜 on 2019/9/28.
//

#include "kernel/types.h"
#include "user/user.h"
#include <stdbool.h>

const int READ_END = -1;
/// int here is not a integer. It was designed to be `std::bool`.
/// `read_pipe_fd` is an fd for read.
void parse_and_find(int read_pipe_fd) {
    int read_flag = 0;
    int stored_prime;   // uninitialized until read_flag is set to 1.
    int child_fds[2]; // uninitialized until read_flag is set to 1.

    while (true) {
        // read digit
        int to_read;
        read(read_pipe_fd, &to_read, sizeof(to_read));
        if (to_read == READ_END) break;
        if (read_flag == 0) {
            read_flag = 1;
            stored_prime = to_read;
            fprintf(2, "prime %d\n", to_read);
            pipe(child_fds);

            if (fork() == 0) {
                // child process
                close(child_fds[1]);
                // handle logic recursively
                parse_and_find(child_fds[0]);
                close(child_fds[0]);
                break;
            } else {
                // father process: close read.
                close(child_fds[0]);
            }
        } else {
            if (to_read % stored_prime != 0) {
                write(child_fds[1], &to_read, sizeof(to_read));
            }
        }
    }

    // destructor
    if (read_flag != 0) {
        write(child_fds[1], &READ_END, sizeof(int));
        wait();
    }
}

int main() {
    // init_fd pipes
    int init_fd[2];
    pipe(init_fd);

    if (fork() != 0) {
        for (int i = 2; i < 35; ++i) {
            write(init_fd[1], &i, sizeof(i));
        }
        write(init_fd[1], &READ_END, sizeof(int)); // send EOF to child process.
        close(init_fd[1]);      // close writes
        wait();
    } else {
        close(init_fd[1]);
        parse_and_find(init_fd[0]);
    }

    exit();
}

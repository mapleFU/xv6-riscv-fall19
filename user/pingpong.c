//
// Created by 付旭炜 on 2019/9/28.
//

#include "kernel/types.h"
#include "user/user.h"

// const int BUF_SIZE = 100;

int main() {
    // preparing pipes
    int parent_fds[2];
    int child_fds[2];

    pipe(parent_fds);
    pipe(child_fds);

    char chat_buf[100];

    if (fork() != 0) {    
	    // father process
        write(parent_fds[1], "p", strlen("p"));
        read(child_fds[0], chat_buf, 1);
        printf("%d: received pong\n", getpid());
    } else {
        // fork is 0, children read from the pipe
        read(parent_fds[0], chat_buf, 1);
	    // fprintf(2, "%d: readed\n", readed);
        printf("%d: received ping\n", getpid());
        write(child_fds[1], "p", strlen("p"));
    }
    exit();
    return 0;
}

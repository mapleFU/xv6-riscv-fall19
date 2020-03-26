// inspired by: https://www.cnblogs.com/nlp-in-shell/p/11909472.html#xargs
#include "kernel/types.h"
#include "user/user.h"

#define true 1
#define false 0

/**
 * xargs echo bye
    hello too
    bye hello too
    ctrl-d
  */
int main(int argc, char *argv[]) {

    char seg_buf[32][32];
    char *seg_ptr[32];
    // seg_ptr = seg_buf;
    for (int i = 0; i < 32; i++) {
        seg_ptr[i] = seg_buf[i];
    }

    for (uint i = 1; i < argc; i++) {
        strcpy(seg_buf[i - 1], argv[i]);
    }

    char buf[1024];
    buf[0] = '\0';
    int current_ptr = 0;
    int seg_buf_ptr = argc - 1;
    int to_break = 0;
    while (!to_break) {
        int r = read(0, buf + current_ptr, 1024 - current_ptr);
        if (r <= 0) {
            to_break = 1;
        }
        if (strlen(buf) == 0) {
            continue;
        }
        // printf("Buf is %s\n", buf);

        buf[r + current_ptr] = '\0';

        while (true) {
            char *line_ptr;
            // find if the new area exists a line
            // If it exists, split the line_ptr.
            // [buf, point) (line_ptr, end]
            line_ptr = strchr(buf + current_ptr, '\n');
            // if (line_ptr != 0) {
            //     printf("InLoop: buf is %s, line_ptr + 1 is %s\n", buf,
            //            line_ptr + 1);
            // } else {
            //     printf("InLoop: buf is %s, line_ptr + 1 is NULL\n", buf);
            // }

            char *cpy_buf = buf;
            while (true) {
                char *ptr;
                ptr = strchr(cpy_buf, ' ');

                if (ptr == 0 || (line_ptr != 0 && ptr - line_ptr > 0)) {
                    if (buf != cpy_buf) {
                        memmove(buf, cpy_buf, strlen(cpy_buf));
                        current_ptr = strlen(buf);
                    }
                    break;
                } else {
                    *ptr = 0;
                    strcpy(seg_buf[seg_buf_ptr], cpy_buf);
                    // move to the next cursor.
                    seg_buf_ptr++;
                    cpy_buf += (ptr - cpy_buf) + 1;
                }
            }

            line_ptr = strchr(buf, '\n');
            if (line_ptr) {
                *line_ptr = 0;
            } else {
                if (to_break) {
                    line_ptr = buf + strlen(buf);
                } else {
                    fprintf(2, "Logic error!");
                }
            }
            if (line_ptr == 0) {
                break;
            } else {

                // excute program and release buf.
                strcpy(seg_buf[seg_buf_ptr], buf);
                seg_buf[seg_buf_ptr + 1][0] = '\0';

                strcpy(buf, line_ptr + 1);
                current_ptr = strlen(buf);

                // printf("Run exec, print args %d:\n", seg_buf_ptr);
                for (int i = 0; i <= seg_buf_ptr; i++) {
                    seg_ptr[i] = seg_buf[i];
                }
                seg_ptr[seg_buf_ptr + 1] = 0;

                // printf("args print over, start exec\n");
                seg_buf_ptr = argc - 1;

                if (fork()) {
                    wait();
                } else {
                    exec(seg_ptr[0], seg_ptr);
                }
            }
        }
    }

    exit();
}

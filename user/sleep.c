//
// Created by 付旭炜 on 2019/9/28.
//

#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        fprintf(2, "sleep: should contains a time\n");
        exit();
    }
    i = atoi(argv[1]);
    sleep(i);
    exit();
}

//
// Created by 付旭炜 on 2019/9/28.
//

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "utils/exceptions.h"
#include <stdbool.h>

/// This method is implemented with brutal force.
/// It try to find if s1 is in s2.
bool instr(const char *s1, const char *s2) {
    int length1 = strlen(s1), length2 = strlen(s2);
    // checks if s1 in s2, so first travers around.
    // it starts at 0, and if length2 - length1 is not valid, it cannot pass.
    for (int i = 0; i < length2 - length1; ++i) {
        // travers s1
        int j;
        for (j = 0; j < length1; ++j) {
            if (s1[j] != s2[i + j]) {
                break;
            }
        }
        if (j == length1) {
            return true;
        }
    }

    return false;
}

const int NAME_BUF_SIZE = 512;

void traverse_dict(int dict_fd, char *w_name_prefix, int already_use,
                   const char *search_name) {
    struct dirent de;
    struct stat st;
    const char *name;
    char *p;

    // start of name_buf.
    const char *buf = w_name_prefix - already_use;

    while (read(dict_fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0)
            continue;
        // skip self and father directory.
        if (!strcmp(de.name, ".") || !strcmp(de.name, "..")) {
            continue;
        }

        // write current filename, it length cannot exists `DIRSIZ`.
        memmove(w_name_prefix, de.name, DIRSIZ);

        // set end of string
        w_name_prefix[DIRSIZ] = 0;
        // stat
        if (stat(buf, &st) < 0) {
            printf("ls: cannot stat %s\n", buf);
            continue;
        }

        switch (st.type) {
        case T_FILE:
            name = fmtname(w_name_prefix);
            if (instr(search_name, name)) {
                // find it
                printf("%s\n", buf);
            }
            break;
        case T_DIR:

            // if it's a directory. Then here it will check if it can be read
            // into buffer..
            if (strlen(buf) + 1 + DIRSIZ + 1 > NAME_BUF_SIZE) {
                printf("ls: path too long\n");
                break;
            }

            int sub_fd;
            if ((sub_fd = open(buf, O_RDONLY)) < 0) {
                fprintf(2, "cannot open %s\n", buf);
                exit();
            }

            // // Note: p means "current write start", it keeps the historical
            // path and make it easy for user to read and find it.
            p = w_name_prefix + strlen(w_name_prefix);
            // add spliter, seems that it don't need to consider windows like
            // spliter
            *p = '/';
            p = p + 1;
            *p = '\0';

            traverse_dict(sub_fd, p, already_use + strlen(w_name_prefix), search_name);

            close(sub_fd);
        }
    }
}

/// ```
/// find . b
/// ```
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(2, "find received %d arguments, request 3\n", argc);
        exit();
    }
    // initialize arguments
    int base_fd;
    char *search_name = argv[2]; // the name to search
    char *root_path = argv[1];
    char *p;

    // intiialize a name buffer
    char buf[NAME_BUF_SIZE];
    buf[0] = '\0';

    const char *name;
    struct stat st;
    // open base dir
    if ((base_fd = open(root_path, O_RDONLY)) < 0) {
        fprintf(2, "ls: cannot open %s\n", root_path);
        exit();
    }

    if (fstat(base_fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", root_path);
        close(base_fd);
        exit();
    }

    switch (st.type) {
    case T_FILE:
        name = fmtname(root_path);
        if (instr(search_name, name)) {
            // find it
            fprintf(2, "%s\n", name);
        }
        break;
    case T_DIR:
        // if it's a directory. Then here it will check if it can be read into
        // buffer..
        if (strlen(root_path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("ls: path too long\n");
            break;
        }
        // cpy from path to buffer.
        strcpy(buf, root_path);
        // Note: p means "current write start", it keeps the historical path and
        // make it easy for user to read and find it.
        p = buf + strlen(buf);
        // add spliter, seems that it don't need to consider windows like
        // spliter
        *p++ = '/';

        traverse_dict(base_fd, p, strlen(buf), search_name);
    }
    close(base_fd);

    exit();
}

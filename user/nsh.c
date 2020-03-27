#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

#define BUFSIZE 100
// char buf[BUFSIZE]; /* buffer for ungetch */
// int bufp = 0;      /* next free position in buf */
char cmd[BUFSIZE];

int getch(char *buf, int length,
          int *bufp) /* get a (possibly pushed-back) character */ {
    if (*bufp >= strlen(buf)) {
        return -1;
    }
    return buf[(*bufp)++];
}

void ungetch(int c, char *buf, int length,
             int *bufp) /* push character back on input */ {
    // printf("ungetch %c with length %d, bufp %d\n", c, length, *bufp);
    if (*bufp > length)
        printf("ungetch: too many characters\n");
    else {
        *bufp = *bufp - 1;
        // buf[*bufp--] = c;
    }
}

enum tokenType {
    PIPE,
    REDIRECT_RIGHT,
    REDIRECT_LEFT,
    LINE,
    NAME,

    EXIT,
};

int is_special_flag(char ch) {
    if (ch <= 0)
        return 0;
    if (ch == '|' || ch == '>' || ch == '<')
        return 1;

    return 0;
}

int read_args(char *buf, int length, int *bufp) {
    char c;
    while ((c = getch(buf, length, bufp)) == ' ' || c == '\t') {
        if (*bufp >= length) {
            return -1;
        }
    }
    if (c <= 0)
        return c;
    if (length <= *bufp) {
        return -1;
    }
    // printf("Read args read %c(length is %d, bufp is %d)\n", c, length,
    // *bufp);
    char *p = cmd;
    for (*p++ = c;;) {
        // printf("Got %c\n", c);
        c = getch(buf, length, bufp);
        int ok =  (c != ' ' && c > 0 && !is_special_flag(c) && *bufp < length);
        if (!ok) break;
        *p++ = c;
    }
    if ( c != ' ' && !is_special_flag(c) && c > 0) {
        *p++ = c;
    }
    

    if (c <= 0) {
        return c;
    }
    *p = '\0';
    if (is_special_flag(c)) {
        ungetch(c, buf, length, bufp);
    }
    return c;
}

int isalpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

int isalphanum(char c) { return isalpha(c) || (c >= '0' && c <= '9'); }

int gettoken(char *buf, int length, int *bufp) /* return next token */ {
    char c;
    char *p = cmd;
    while ((c = getch(buf, length, bufp)) == ' ' || c == '\t')
        ;
    if (c <= 0) {
        return EXIT;
    }
    if (c == '|') {
        return PIPE;
    } else if (c == '>') {
        return REDIRECT_RIGHT;
    } else if (c == '<') {
        return REDIRECT_LEFT;
    } else if (c == '\n') {
        return LINE;
    } else if (isalpha(c)) {
        // note: c = getch cannot be moved to final
        for (*p++ = c; isalphanum(c = getch(buf, length, bufp));) {
            // printf("[gettoken] Fetch %c\n", c);
            *p++ = c;
        }
        // printf("[gettoken] Final %c\n", c);
        *p = '\0';
        ungetch(c, buf, length, bufp);
        return NAME;
    } else {
        fprintf(2, "don't know the input!");
        return EXIT;
    }
}

const int MAX_NAME_LENGTH = 32;
const int MAX_ARGS = 6;

void run_cmd(char *buf);

int main(int argc, char *argv[]) {
    char buf[BUFSIZE];
    while (1) {
        printf("@ ");
        gets(buf, BUFSIZE - 1);
        if (buf[0] == 0) // EOF
            exit(0);
        *strchr(buf, '\n') = '\0';

        if (fork() == 0) {
            run_cmd(buf);
        } else {
            wait(0);
        }
    }
}

// First: split pipe.
// Second: handle redirect.
// Third: exec.
void run_cmd(char *buf) {
    char *pipe_s;
    if ((pipe_s = strchr(buf, '|'))) {
        // split by pipe.
        *pipe_s = '\0';
        // char *to = pipe + 1;
        // fork and redirect.
        int p[2];
        pipe(p);
        if (fork() == 0) {
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            run_cmd(pipe_s + 1);
            return;
        } else {
            close(1);
            dup(p[1]);
            close(p[0]);
            close(p[1]);
        }
    }

    char *redirect;
    char filename[BUFSIZE];

    if ((redirect = strchr(buf, '>'))) {
        *redirect = '\0';
        char *to = redirect + 1;
        int to_bufp = 0;
        read_args(to, strlen(to), &to_bufp);
        strcpy(filename, cmd);
        // printf("> to filename %s\n", filename);
        close(1);
        if (open(filename, O_WRONLY|O_CREATE) <= 0) {
            fprintf(2, "cannot open %s\n", filename);
        }
        // close(fd);
    }

    if ((redirect = strchr(buf, '<'))) {
        *redirect = '\0';
        char *to = redirect + 1;
        int to_bufp = 0;
        read_args(to, strlen(to), &to_bufp);
        strcpy(filename, cmd);
        // printf("< from filename %s\n", filename);
        close(0);
        // TODO: if == 0, it will raise an error, please find out the reason.
        if (open(filename, O_RDONLY) < 0) {
            fprintf(2, "cannot open %s\n", filename);
        }
    }

    // exec logic

    int bufp = 0;
    // printf("Run gettoken\n");
    gettoken(buf, strlen(buf), &bufp);
    // printf("After gettoken: %s\n", buf + bufp);

    char exec_name[BUFSIZE];
    char exec_args[6][BUFSIZE];
    char *exec_args_ptr[BUFSIZE + 1];
    strcpy(exec_name, cmd);
    strcpy(exec_args[0], cmd);
    exec_args_ptr[0] = exec_args[0];
    // printf("exec_name is %s\n", exec_name);
    int current_arg = 1;
    while (1) {
        // printf("Run read_args\n");
        if (read_args(buf, strlen(buf), &bufp) < 0) {
            break;
        }
        strcpy(exec_args[current_arg], cmd);
        exec_args_ptr[current_arg] = exec_args[current_arg];
        ++current_arg;
    }
    exec_args_ptr[current_arg] = 0;

    exec(exec_name, exec_args_ptr);
}

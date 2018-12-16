#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char **environ;

#include <stdio.h>

#define BUFFER_LEN 100
#define PROMPT_COLOR "60"

const char* PROMPT_FORMAT = "\033[48;5;" PROMPT_COLOR "m %s \033[0m"
                         "\033[38;5;" PROMPT_COLOR "m\ue0b0\033[0m ";
const char* ERROR_FORMAT = "\033[31mError: %s\033[0m\n";

static char latex_call[BUFFER_LEN] = {0};

static void print_prompt(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[BUFFER_LEN];
    vsnprintf(buffer, BUFFER_LEN, fmt, args);
    printf(PROMPT_FORMAT, buffer);
    fflush(stdout);

    va_end(args);
}

static void print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[BUFFER_LEN];
    vsnprintf(buffer, BUFFER_LEN, fmt, args);
    printf(ERROR_FORMAT, buffer);
    fflush(stdout);

    va_end(args);
}

int get_workspace(char *buffer) {
    int fd[2];
    if (pipe(fd)) {
        return 1;
    }

    // get current workspace
    int original_stdout = dup(STDOUT_FILENO);
    if (dup2(fd[1], STDOUT_FILENO) < 0) {
        // close pipe
        close(fd[0]);
        close(fd[1]);

        return 1;
    }

    int error = 0;

    if (fork() == 0) {
        char *cmd_line[] = {
            latex_call, "workspace", NULL
        };
        execve(cmd_line[0], cmd_line, NULL);

        // failure to run command
        error = 1;
        exit(error);
    } else {
        // wait for child process to finish
        int child_status;
        waitpid(-1, &child_status, 0);

        char buf[BUFFER_LEN] = {0};
        bool child_error = child_status ||
                           read(fd[0], buf, BUFFER_LEN - 1) <= 0 ||
                           sscanf(buf, "Current workspace: %s", buffer) <= 0;
        if (child_error) {
            error = 1;
        }
    }

    // reset standard out
    dup2(original_stdout, STDOUT_FILENO);

    // clean up file descriptors
    close(fd[0]);
    close(fd[1]);
    close(original_stdout);

    return error;
}

int main(int argc, char *argv[]) {

    extern char *optarg;

    bool help = false;

    char opt;
    while ((opt = getopt(argc, argv, "hc:")) != -1) {
        switch (opt) {
        case 'h':
            help = true;
            break;
        case 'c':
            strncpy(latex_call, optarg, BUFFER_LEN - 1);
            latex_call[strlen(optarg)] = '\0';
            break;
        }
    }

    if (help || argc == 1) {
        printf("usage: latex -c absolute_path_to_latex_bash_script "
               "[-r command_to_run]\n");
        return 0;
    }

    if (strlen(latex_call) == 0) {
        print_error("No latex script given. See usage.");
    }

    char buffer[BUFFER_LEN];
    while (true) {
        // get workspace name
        if (get_workspace(buffer)) {
            print_error("Could not get workspace name.");
            return 0;
        }

        print_prompt(buffer);

        bool read_error = (fgets(buffer, BUFFER_LEN, stdin) == NULL) &&
                          ferror(stdin);
        if (read_error) {
            print_error("Can't read from standard input.");
        }

        if (feof(stdin)) {
            // control-d
            printf("\n");
            return 0;
        }

        if (fork() == 0) {
            char *test[] = {"/usr/bin/vim", NULL};
            execve(test[0], test, NULL);
            return 0;
        } else {
            int status;
            waitpid(-1, &status, 0);
        }
    }
}

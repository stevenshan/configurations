#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "linked_list.h"

#define BUFFER_LEN 100
#define PROMPT_COLOR "60"

extern char **environ;
static char latex_call[BUFFER_LEN] = {0};
static linked_list *jobs = NULL;

static void sigchld_handler(int sig);

const char* PROMPT_FORMAT = "\033[48;5;" PROMPT_COLOR "m %s \033[0m"
                         "\033[38;5;" PROMPT_COLOR "m\ue0b0\033[0m ";
const char* ERROR_FORMAT = "\033[31mError: %s\033[0m\n";
const char* WARNING_FORMAT = "\033[31m%s\033[0m\n";

sigset_t get_sig_mask() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    sigaddset(&mask, SIGCHLD);
    return mask;
}

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

static int get_workspace(char *buffer) {
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

static void remove_job(int pid) {

}

static void break_arguments(char *cmdline, char **buffer) {
    size_t i;
    size_t j = 0;
    bool quote_active = false;
    char quote_match = 0;
    char c;
    char *start = NULL;
    size_t len = strlen(cmdline);

    for (i = 0; i < len; i++) {
        c = cmdline[i];
        if (c == ' ' && start == NULL) {
            continue;
        } else if (c == ' ' && !quote_active) {
            buffer[j] = start;
            j += 1;
            cmdline[i] = '\0';
            start = NULL;
        } else if (quote_active && c == quote_match) {
            quote_active = false;
        } else if (!quote_active && (c == '\'' || c == '"')) {
            quote_active = true;
            quote_match = c;
            if (start == NULL) {
                start = &cmdline[i];
            }
        } else if (c != ' ' && start == NULL) {
            start = &cmdline[i];
        }
    }

    if (start != NULL) {
        buffer[j] = start;
        cmdline[i] = '\0';
    }
}

static void exec_command(char* buffer) {
    char command[BUFFER_LEN];

    // find command name
    if (sscanf(buffer, "%s", command) != 1) {
        // needs to at least match 1 item, ie. command
        return;
    }

    if (strcmp(command, "preview")) {
    } else {
        printf("preview\n");
    }

    if (fork() == 0) {

        char *cmd[BUFFER_LEN] = {NULL};
        cmd[0] = latex_call;

        break_arguments(buffer, &cmd[1]);

        execve(cmd[0], cmd, NULL);

        exit(0);
    }
    wait(NULL);
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
        return 1;
    }

    if ((jobs = new_linked_list()) == NULL) {
        print_error("Could not initialize job list.");
        return 1;
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

        // remove trailing newline character
        if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

        exec_command(buffer);
    }
}

static void sigchld_handler(int sig) {
    int _errno = errno;

    pid_t pid;
    sigset_t mask, prev;
    int status;
    mask = get_sig_mask();
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        sigprocmask(SIG_BLOCK, &mask, &prev);

        remove_job(pid);

        sigprocmask(SIG_SETMASK, &prev, NULL);
    }

    errno = _errno;
}

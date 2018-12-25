#define _POSIX_C_SOURCE 200809L

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
#include <termios.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "linked_list.h"
#include "latex.h"

#define PROMPT_COLOR "60"

extern char **environ;
static struct termios terminal_settings;
static int original_stdout;
static char latex_call[BUFFER_LEN] = {0};
static linked_list *jobs = NULL;
static pid_t fg_pid = 0;
static int job_id = 0;

typedef void handler_t(int);
static void sigchld_handler(int sig);
static void sigint_handler(int sig);
static void sigtstp_handler(int sig);
static void cleanup();

const char* PROMPT_FORMAT = "\033[48;5;" PROMPT_COLOR "m %s \033[0m"
                         "\033[38;5;" PROMPT_COLOR "m\ue0b0\033[0m ";
const char* ERROR_FORMAT = "\033[31mError: %s\033[0m\n";
const char* WARNING_FORMAT = "\033[31m%s\033[0m\n";

static sigset_t get_sig_mask() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    sigaddset(&mask, SIGCHLD);
    return mask;
}

static void Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    sigaction(signum, &action, &old_action);
}

static void reset_signal_handlers() {
    Signal(SIGCHLD, SIG_DFL);
    Signal(SIGINT, SIG_DFL);
    Signal(SIGTSTP, SIG_DFL);
}

static void reset_terminal_settings() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settings);
    tcsetpgrp(STDIN_FILENO, getpid());
    clearerr(stdin);
}

static void get_prompt(char *buffer, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char temp[BUFFER_LEN];
    vsnprintf(temp, BUFFER_LEN, fmt, args);
    snprintf(buffer, BUFFER_LEN, PROMPT_FORMAT, temp);

    va_end(args);
}

static void print_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[BUFFER_LEN];
    vsnprintf(buffer, BUFFER_LEN, fmt, args);
    fprintf(stderr, ERROR_FORMAT, buffer);
    fflush(stderr);

    va_end(args);
}

static int exec_latex_call(char *buffer, size_t n, char **args) {
    int fd[2];
    if (pipe(fd)) {
        return 1;
    }

    // get current workspace
    if (dup2(fd[1], STDOUT_FILENO) < 0) {
        // close pipe
        close(fd[0]);
        close(fd[1]);

        return 1;
    }

    int error = 0;
    sigset_t mask, prev;
    mask = get_sig_mask();
    sigprocmask(SIG_BLOCK, &mask, &prev);

    pid_t pid = fork();
    if (pid == 0) {
        reset_signal_handlers();
        sigprocmask(SIG_SETMASK, &prev, NULL);

        execve(args[0], args, environ);

        // failure to run command
        error = 1;
        exit(error);
    } else {
        // wait for child process to finish
        int child_status;
        waitpid(pid, &child_status, 0);
        sigprocmask(SIG_SETMASK, &prev, NULL);

        ssize_t len;
        if (child_status || (len = read(fd[0], buffer, n -1)) <= 0) {
            error = 1;
        } else {
            buffer[len] = '\0';
        }
    }

    // reset standard out
    dup2(original_stdout, STDOUT_FILENO);

    // clean up file descriptors
    close(fd[0]);
    close(fd[1]);

    return error;
}

static int get_workspace(char *buffer) {
    char *cmd_line[] = {
        latex_call, "workspace", NULL
    };

    int result = exec_latex_call(buffer, BUFFER_LEN, cmd_line);
    return result || sscanf(buffer, "Current workspace: %s", buffer) != 1;
}

static void break_arguments(char *cmdline, char **buffer, size_t *cnt) {
    size_t i;
    size_t j = 0;
    bool quote_active = false;
    char quote_match = 0;
    char c;
    char *start = NULL;
    size_t len = strlen(cmdline);
    size_t count = 0;

    for (i = 0; i < len; i++) {
        c = cmdline[i];
        if (c == ' ' && start == NULL) {
            continue;
        } else if (c == ' ' && !quote_active) {
            buffer[j] = start;
            j += 1;
            cmdline[i] = '\0';
            start = NULL;
            count += 1;
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
        count += 1;
    }

    if (cnt != NULL) {
        *cnt = count;
    }
}

static bool str_to_pid(char* str, int *value) {
    if (value == NULL || str == NULL) {
        return false;
    }

    if (sscanf(str, "%%%d", value)) {
        // if job number given

        node *n = find_i(jobs, *value);
        if (n == NULL) {
            *value = 0;
        } else {
            *value = n->id;
        }
        return true;
    }

    if (sscanf(str, "%d", value)) {
        // if pid number given
        return true;
    }

    return false;
}

static void print_job(int i, int pid, char *key) {
    printf("%-5d %-7d %s\n", i, pid, key);
}

static void print_jobs() {
    if (linked_list_len(jobs)) {
        printf("%-5s %-7s %s\n", "ID", "PID", "Command");
        map_list(jobs, print_job);
    } else {
        printf("No jobs running.\n");
    }
}

static bool streq(const char *s1, const char *s2) {
    return !strcmp(s1, s2);
}

static void set_fg_process(pid_t pid) {
    sigset_t mask, prev;
    mask = get_sig_mask();
    sigprocmask(SIG_BLOCK, &mask, &prev);

    // parent process
    tcsetpgrp(STDIN_FILENO, pid);
    fg_pid = pid;

    sigset_t empty_mask;
    sigemptyset(&empty_mask);

    while (fg_pid) {
        sigsuspend(&empty_mask);
    }

    sigprocmask(SIG_SETMASK, &prev, NULL);
}

static void exec_command(char* buffer) {
    char command[BUFFER_LEN];

    // find command name
    if (sscanf(buffer, "%s", command) != 1) {
        // needs to at least match 1 item, ie. command
        return;
    }

    char command_line[BUFFER_LEN];
    strncpy(command_line,  buffer, BUFFER_LEN);

    char *cmd[BUFFER_LEN] = {NULL};
    size_t num_arguments;
    cmd[0] = latex_call;
    break_arguments(buffer, &cmd[1], &num_arguments);

    sigset_t mask, prev;
    mask = get_sig_mask();
    sigprocmask(SIG_BLOCK, &mask, &prev);

    if (streq(command, "exit")) {
        exit(0);
    } else if (streq(command, "jobs")) {
        print_jobs();

        sigprocmask(SIG_SETMASK, &prev, NULL);
        return;
    } else if (streq(command, "fg")) {
        int arg;
        node *pnode;
        if (str_to_pid(cmd[2], &arg)) {
            if (id_in_linked_list(jobs, arg)) {
                kill(-arg, SIGCONT);
                set_fg_process(arg);
            } else {
                printf("%s: no such job\n", command_line);
            }
        } else if ((pnode = get_recently_accessed(jobs)) != NULL) {
            kill(-pnode->id, SIGCONT);
            set_fg_process(pnode->id);
        } else {
            printf("Usage: fg [pid] [%%jid]\n");
        }

        sigprocmask(SIG_SETMASK, &prev, NULL);
        return;
    } else if (streq(command, "kill")) {
        int arg;
        if (str_to_pid(cmd[2], &arg)) {
             if (id_in_linked_list(jobs, arg)) {
                kill(-arg, SIGTERM);
            } else {
                printf("%s: no such job\n", command_line);
             }
        } else {
            printf("Usage: kill [pid] [%%jid]\n");
        }

        sigprocmask(SIG_SETMASK, &prev, NULL);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        print_error("Couldn't fork process.");
    } else if (pid == 0) {
        // child process
        setpgid(0, 0);

        reset_signal_handlers();
        sigprocmask(SIG_SETMASK, &prev, NULL);

        execve(cmd[0], cmd, environ);

        exit(0);
    } else {
        // parent process
        job_id += 1;
        add_node(jobs, job_id, pid, command_line);

        set_fg_process(pid);

        sigprocmask(SIG_SETMASK, &prev, NULL);
    }
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
    } else if (strlen(latex_call) == 0) {
        print_error("No latex script given. See usage.");
        return 1;
    } else if ((jobs = new_linked_list()) == NULL) {
        print_error("Could not initialize job list.");
        return 1;
    } else if (atexit(cleanup) < 0) {
        print_error("Couldn't register cleanup function.");
        return 1;
    } else if ((original_stdout = dup(STDOUT_FILENO)) < 0) {
        print_error("Couldn't duplicate original stdout file descriptor.");
        return 1;
    } else if (tcgetattr(STDIN_FILENO, &terminal_settings)) {
        print_error("Couldn't get terminal settings.");
        return 1;
    }

    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGINT, sigint_handler);
    Signal(SIGTSTP, sigtstp_handler);
    Signal(SIGTTOU, SIG_IGN);
    Signal(SIGTTIN, SIG_IGN);

    char buffer[BUFFER_LEN];
    char *cmd_buffer = NULL;
    while (true) {
        // get workspace name
        if (get_workspace(buffer)) {
            print_error("Could not get workspace name.");
            return 0;
        }

        get_prompt(buffer, buffer);
        cmd_buffer = readline(buffer);
        add_history(cmd_buffer);

        if (cmd_buffer == NULL) {
            printf("\n");
            return 0;
        } else {
            // remove trailing newline character
            if (strlen(cmd_buffer) > 0 && cmd_buffer[strlen(buffer) - 1] == '\n') {
                cmd_buffer[strlen(cmd_buffer) - 1] = '\0';
            }

            exec_command(cmd_buffer);

            free(cmd_buffer);
        }
    }
}

static node max_node(node n1, node n2) {
    node result;
    result.i = n1.i > n2.i ? n1.i : n2.i;
    return result;
}

static void sigchld_handler(int sig) {
    int _errno = errno;

    pid_t pid;
    sigset_t mask, prev;
    int status;
    mask = get_sig_mask();

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        sigprocmask(SIG_BLOCK, &mask, &prev);

        bool is_foreground = fg_pid == pid;
        bool stopped = WIFSTOPPED(status);

        if (is_foreground) {
            fg_pid = 0;
        }
        if (!stopped) {
            delete_node_id(jobs, pid, NULL);

            // recalculate job id count
            node acc;
            acc.i = 0;
            acc = reduce_list(jobs, &max_node, acc);
            job_id = acc.i;
        } else {
            node *pnode;
            if ((pnode = find_id(jobs, pid)) != NULL) {
                printf("\nJob [%d] (%d) '%s' stopped.\n",
                       pnode->i, pnode->id, pnode->key);
            }
        }

        if (!fg_pid) {
            reset_terminal_settings();
        }

        sigprocmask(SIG_SETMASK, &prev, NULL);
    }

    errno = _errno;
}

static void sigint_handler(int sig) {
}

static void sigtstp_handler(int sig) {
}

static void cleanup_pid(int i, int pid, char *key) {
    kill(-pid, SIGCONT);
    kill(-pid, SIGTERM);
}

static void cleanup() {
    map_list(jobs, &cleanup_pid);
    while (waitpid(-1, NULL, 0) > 0);

    close(original_stdout);
    reset_signal_handlers();
    destroy_linked_list(jobs);
    reset_terminal_settings();
}

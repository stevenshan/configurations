/*
 * sh_helper.c - copied from starter code for tshlab 15-213 CMU
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "sh_helper.h"

// Parsing states, used internally in parseline
typedef enum parse_state
{
    ST_NORMAL,
    ST_INFILE,
    ST_OUTFILE
} parse_state;

/*
 * parseline - Parse the command line and build the argv array.
 * Not async-signal-safe.
 */
parseline_return parseline(const char *cmdline,
                           struct cmdline_tokens *token) {
    const char delims[] = " \t\r\n";    // argument delimiters (white-space)
    char *buf;                          // ptr that traverses command line
    char *next;                         // ptr to the end of the current arg
    char *endbuf;                       // ptr to end of cmdline string

    parse_state parsing_state;          // indicates if the next token is the
                                        // input or output file

    if (cmdline == NULL) {
        return PARSELINE_EMPTY;
    }

    strncpy(token->_buf, cmdline, MAXLINE_TSH);
    token->_buf[MAXLINE_TSH-1] = '\0';

    buf = token->_buf;
    endbuf = buf + strlen(buf);

    // initialize default values
    token->argc = 0;
    token->infile = NULL;
    token->outfile = NULL;

    /* Build the argv list */
    parsing_state = ST_NORMAL;

    while (buf < endbuf) {
        /* Skip the white-spaces */
        buf += strspn(buf, delims);
        if (buf >= endbuf) break;

        /* Check for I/O redirection specifiers */
        if (*buf == '<') {
            if (token->infile) {    // infile already exists
                return PARSELINE_ERROR;
            }
            parsing_state = ST_INFILE;
            buf++;
            continue;
        } else if (*buf == '>') {
            if (token->outfile) {   // outfile already exists
                return PARSELINE_ERROR;
            }
            parsing_state = ST_OUTFILE;
            buf++;
            continue;
        } else if (*buf == '\'' || *buf == '\"') {
            /* Detect quoted tokens */
            buf++;
            next = strchr(buf, *(buf - 1));
        } else {
            /* Find next delimiter */
            next = buf + strcspn(buf, delims);
        }

        if (next == NULL) {
            /* Returned by strchr(); this means that the closing
               quote was not found. */
            return PARSELINE_ERROR;
        }

        /* Terminate the token */
        *next = '\0';

        /* Record the token as either the next argument or the i/o file */
        switch (parsing_state) {
        case ST_NORMAL:
            token->argv[token->argc] = buf;
            token->argc = token->argc + 1;
            break;
        case ST_INFILE:
            token->infile = buf;
            break;
        case ST_OUTFILE:
            token->outfile = buf;
            break;
        default:
            fprintf(stderr, "Error: Ambiguous I/O redirection\n");
            return PARSELINE_ERROR;
        }
        parsing_state = ST_NORMAL;

        /* Check if argv is full */
        if (token->argc >= MAXARGS - 1) break;

        buf = next + 1;
    }

    if (parsing_state != ST_NORMAL) { // buf ends with < or >
        fprintf(stderr, "Error: must provide file name for redirection\n");
        return PARSELINE_ERROR;
    }

    /* The argument list must end with a NULL pointer */
    token->argv[token->argc] = NULL;

    if (token->argc == 0) {  /* ignore blank line */
        return PARSELINE_EMPTY;
    }

    if ((strcmp(token->argv[0], "quit")) == 0) {        /* quit command */
        token->builtin = BUILTIN_QUIT;
    } else if ((strcmp(token->argv[0], "jobs")) == 0) { /* jobs command */
        token->builtin = BUILTIN_JOBS;
    } else if ((strcmp(token->argv[0], "bg")) == 0) {   /* bg command */
        token->builtin = BUILTIN_BG;
    } else if ((strcmp(token->argv[0], "fg")) == 0) {   /* fg command */
        token->builtin = BUILTIN_FG;
    } else {
        token->builtin = BUILTIN_NONE;
    }

    // Returns 1 if job runs on background; 0 if job runs on foreground

    if (*token->argv[(token->argc)-1] == '&') {
        token->argv[--(token->argc)] = NULL;
        return PARSELINE_BG;
    } else {
        return PARSELINE_FG;
    }
}

#ifndef __SH_HELPER_H__
#define __SH_HELPER_H__

#include <unistd.h>
#include <stdbool.h>

#define MAXLINE_TSH  1024   /* max line size */
#define MAXARGS       128   /* max args on a command line */
#define MAXJOBS        64   /* max jobs at any point in time */

/*
 * Job states: FG (foreground), BG (background), ST (stopped),
 *             UNDEF (undefined)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */
typedef enum job_state
{
    UNDEF,    // Undefined (do not use)
    FG,       // Foreground job
    BG,       // Background job
    ST        // Stopped job
} job_state;


/* Parseline return states indicating the type of cmdline that was parsed */
typedef enum parseline_return
{
    PARSELINE_FG,       // Foreground job
    PARSELINE_BG,       // Background job
    PARSELINE_EMPTY,    // Empty cmdline
    PARSELINE_ERROR     // Parse error
} parseline_return;


/* Types of builtins that can be executed by the shell */
typedef enum builtin_state
{
    BUILTIN_NONE,       // Not a builtin command
    BUILTIN_QUIT,       // quit (exit the shell)
    BUILTIN_JOBS,       // jobs (list running jobs)
    BUILTIN_BG,         // bg (run job in background)
    BUILTIN_FG          // fg (run job in foreground)
} builtin_state;


/* Result of parsing a command line from parseline */
struct cmdline_tokens
{
    int argc;                 // Number of arguments passed
    char *argv[MAXARGS];      // The arguments list
    char *infile;             // The filename for input redirection, or NULL
    char *outfile;            // The filename for output redirection, or NULL
    builtin_state builtin;    // Indicates if argv[0] is a builtin command
    char _buf[MAXLINE_TSH];   // Internal backing buffer (do not use)
};


parseline_return parseline(const char *cmdline,
                           struct cmdline_tokens *token);

#endif // __TSH_HELPER_H__

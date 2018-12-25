#ifndef __LATEX__
#define __LATEX__

#include <limits.h>

#define BUFFER_LEN 100

#define CMD_HISTORY_LEN 5

#define MAX_PATH_LEN PATH_MAX

#define PROMPT_COLOR "60"
#define PATH_COLOR "238"

#define PROMPT_FORMAT "\033[48;5;" PROMPT_COLOR "m %s \033[0m"
#define PATH_FORMAT "\033[48;5;" PATH_COLOR "m %s \033[0m"
#define ERROR_FORMAT "\033[31mError: %s\033[0m\n"
#define WARNING_FORMAT "\033[31m%s\033[0m\n"

#endif

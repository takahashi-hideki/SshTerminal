#include "ssh_debug.h"

#include <stdio.h>

void debug_log(int error_code, char* error_msg)
{
#ifdef _SSH_DEBUG
    fprintf(stderr, "%s\n", error_msg);
    fprintf(stderr, "error code : %d\n", error_code);
#endif
    return;
}
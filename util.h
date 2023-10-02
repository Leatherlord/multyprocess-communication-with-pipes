#ifndef __IFMO_DISTRIBUTED_CLASS_UTIL_H
#define __IFMO_DISTRIBUTED_CLASS_UTIL_H

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

enum RESULT_CODE
{
    SUCCESS,
    UNCKNOWN_FAIL,
    WRONG_ARGUMENTS,
    ADDRESSATION_FAIL,
    MALLOC_FAIL,
    PIPE_FAIL,
    PIPE_READ_FAIL,
    PIPE_WRITE_FAIL,
    FILE_OPEN_FAIL,
    FILE_CLOSE_FAIL,
    FILE_WRITE_FAIL,
    BUFFER_WRITE_FAIL,
    NO_MSG_READY,
    FORK_FAIL,
    WRONG_MESSAGE,
    WAIT_FAIL
};

typedef enum RESULT_CODE RESULT_CODE;

const char *result_string(RESULT_CODE code);

#endif

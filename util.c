#include "util.h"

const char *result_string(RESULT_CODE code)
{
    switch (code)
    {
    case SUCCESS:
        return "SUCCESS";
    case WRONG_ARGUMENTS:
        return "WRONG_ARGUMENTS";
    case ADDRESSATION_FAIL:
        return "ADDRESSATION_FAIL";
    case MALLOC_FAIL:
        return "MALLOC_FAIL";
    case PIPE_FAIL:
        return "PIPE_FAIL";
    case FILE_OPEN_FAIL:
        return "FILE_OPEN_FAIL";
    case FILE_WRITE_FAIL:
        return "FILE_WRITE_FAIL";
    case BUFFER_WRITE_FAIL:
        return "BUFFER_WRITE_FAIL";

    default:
        return "UNCKNOWN_FAIL";
    }
}

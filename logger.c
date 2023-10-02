#include "logger.h"

int events_fd = -1;
int pipes_fd = -1;

RESULT_CODE open_files()
{
    events_fd = open(events_log, O_TRUNC | O_WRONLY | O_CREAT, 0666);
    pipes_fd = open(pipes_log, O_TRUNC | O_WRONLY | O_CREAT, 0666);

    if (events_fd == -1 || pipes_fd == -1)
    {
        perror(result_string(FILE_OPEN_FAIL));
        return FILE_OPEN_FAIL;
    }
    return SUCCESS;
}

RESULT_CODE close_files()
{
    if (events_fd == -1 || pipes_fd == -1)
    {
        return FILE_CLOSE_FAIL;
    }
    if (close(events_fd) == -1 || close(pipes_fd) == -1)
    {
        perror(result_string(FILE_CLOSE_FAIL));
        return FILE_CLOSE_FAIL;
    }
    return SUCCESS;
}

RESULT_CODE private_log(int fd, char *buffer, size_t count)
{
    int acc = 0;
    int result;
    if (fd == -1)
    {
        return FILE_OPEN_FAIL;
    }
    while (acc < count)
    {
        result = write(fd, buffer + acc, count - acc);
        if (result == -1)
        {
            return FILE_WRITE_FAIL;
        }
        acc += result;
    }
    return SUCCESS;
}

RESULT_CODE event_log(char *buffer, size_t count)
{
    return private_log(events_fd, buffer, count);
}

RESULT_CODE pipe_log(char *buffer, size_t count)
{
    return private_log(pipes_fd, buffer, count);
}

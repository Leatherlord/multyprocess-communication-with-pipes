#include "logger.h"
#include "ipc.h"
#include "pa1.h"
#include "util.h"
#include "grid.h"

#include <wait.h>

#define BUFFER_SIZE 1024

RESULT_CODE wait_from_all(grid_id_pair *pair, MessageType type)
{
    RESULT_CODE result;
    Message message;

    channel_grid *grid = pair->grid;
    local_id id = pair->id;

    for (size_t i = 1; (int)i < grid->width; i++)
    {
        if ((local_id)i == id)
        {
            continue;
        }
        result = NO_MSG_READY;
        while (result)
        {
            result = receive((void *)pair, i, &message);
            if (result && result != NO_MSG_READY)
            {
                return result;
            }
        }
        if (message.s_header.s_type != type)
        {
            return WRONG_MESSAGE;
        }
    }

    return SUCCESS;
}

RESULT_CODE process_routine(grid_id_pair *pair)
{
    RESULT_CODE result;
    char buffer[BUFFER_SIZE];
    int chars_written;
    Message message;

    channel_grid *grid = pair->grid;
    local_id id = pair->id;

    result = close_unused_pipes(pair);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not close unused pipes");
        fprintf(stderr, "%s\n", result_string(result));
        close_all_pipes(grid);
        free_grid(&grid);

        return result;
    }

    chars_written = sprintf(buffer, log_started_fmt, id, getpid(), getppid());
    if (chars_written < 0)
    {
        return BUFFER_WRITE_FAIL;
    }

    result = event_log(buffer, chars_written);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not log event\n");
        return result;
    }

    message.s_header.s_magic = MESSAGE_MAGIC;
    message.s_header.s_payload_len = chars_written;
    message.s_header.s_type = STARTED;
    memcpy(&(message.s_payload), buffer, chars_written + 1);

    result = send_multicast((void *)pair, &message);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not send multicast");
        fprintf(stderr, "%s\n", result_string(result));
        return result;
    }

    result = wait_from_all(pair, STARTED);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not receive STARTED messages from siblings");
        fprintf(stderr, "%s\n", result_string(result));
        return result;
    }

    chars_written = sprintf(buffer, log_received_all_started_fmt, id);
    if (chars_written < 0)
    {
        return BUFFER_WRITE_FAIL;
    }

    result = event_log(buffer, chars_written);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not log event\n");
        return result;
    }

    sleep(2);

    chars_written = sprintf(buffer, log_done_fmt, id);
    if (chars_written < 0)
    {
        return BUFFER_WRITE_FAIL;
    }

    result = event_log(buffer, chars_written);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not log event\n");
        return result;
    }

    message.s_header.s_magic = MESSAGE_MAGIC;
    message.s_header.s_payload_len = chars_written;
    message.s_header.s_type = DONE;
    memcpy(&(message.s_payload), buffer, chars_written + 1);

    result = send_multicast((void *)pair, &message);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not send multicast");
        fprintf(stderr, "%s\n", result_string(result));
        return result;
    }

    result = wait_from_all(pair, DONE);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not receive DONE messages from siblings");
        fprintf(stderr, "%s\n", result_string(result));
        return result;
    }

    chars_written = sprintf(buffer, log_received_all_done_fmt, id);
    if (chars_written < 0)
    {
        return BUFFER_WRITE_FAIL;
    }

    result = event_log(buffer, chars_written);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not log event\n");
        return result;
    }

    return SUCCESS;
}

int main(int argc, const char *argv[])
{
    RESULT_CODE result;
    if (argc < 3 || strcmp(argv[1], "-p") != 0)
    {
        printf("+======================================+\n");
        printf("|./<name_of_file> -p <num_of_processes>|\n");
        printf("+======================================+\n");
        return WRONG_ARGUMENTS;
    }
    int processes = atoi(argv[2]);
    result = open_files();
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not open files\n");
        fprintf(stderr, "%s\n", result_string(result));
        return result;
    }
    channel_grid *grid;

    result = allocate_grid(processes + 1, &grid);

    if (result)
    {
        fprintf(stderr, "%s\n", "Could not allocate grid\n");
        fprintf(stderr, "%s\n", result_string(result));
        free_grid(&grid);

        return result;
    }

    result = fill_grid(grid);

    if (result)
    {
        fprintf(stderr, "%s\n", "Could not fill grid");
        fprintf(stderr, "%s\n", result_string(result));
        close_all_pipes(grid);
        free_grid(&grid);

        return result;
    }

    for (size_t i = 0; (int)i < processes; i++)
    {
        pid_t fork_result = fork();
        if (fork_result == -1)
        {
            fprintf(stderr, "%s\n", "Could not fork");
            perror(result_string(FORK_FAIL));
            close_all_pipes(grid);
            free_grid(&grid);
            return FORK_FAIL;
        }
        if (fork_result == 0)
        {
            grid_id_pair pair;
            pair.grid = grid;
            pair.id = (local_id)i + 1;
            return process_routine(&pair);
        }
    }

    grid_id_pair pair;
    pair.grid = grid;
    pair.id = 0;

    result = close_unused_pipes(&pair);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not close unused pipes");
        fprintf(stderr, "%s\n", result_string(result));
        close_all_pipes(grid);
        free_grid(&grid);

        return result;
    }

    result = wait_from_all(&pair, STARTED);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not receive STARTED messages from children");
        fprintf(stderr, "%s\n", result_string(result));
        close_all_pipes(grid);
        free_grid(&grid);
        return result;
    }

    result = wait_from_all(&pair, DONE);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not receive DONE messages from children");
        fprintf(stderr, "%s\n", result_string(result));
        close_all_pipes(grid);
        free_grid(&grid);
        return result;
    }

    for (size_t i = 0; (int)i < processes; i++)
    {
        pid_t wait_result = wait(NULL);
        if (wait_result == -1)
        {
            fprintf(stderr, "%s\n", "Could not wait for all children");
            fprintf(stderr, "%s\n", result_string(WAIT_FAIL));
            close_all_pipes(grid);
            free_grid(&grid);
            return WAIT_FAIL;
        }
    }

    result = close_all_pipes(grid);
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not close pipes");
        fprintf(stderr, "%s\n", result_string(result));
        free_grid(&grid);

        return result;
    }
    free_grid(&grid);

    result = close_files();
    if (result)
    {
        fprintf(stderr, "%s\n", "Could not close log files");
        fprintf(stderr, "%s\n", result_string(result));

        return result;
    }

    return SUCCESS;
}

#include "grid.h"

RESULT_CODE allocate_grid(size_t width, channel_grid **destination)
{
    *destination = (channel_grid *)malloc(sizeof(channel_grid));
    (*destination)->width = width;
    if (!destination)
    {
        return MALLOC_FAIL;
    }
    (*destination)->grid = (channel **)malloc(sizeof(channel *) * width * width);
    if (!(*destination)->grid)
    {
        return MALLOC_FAIL;
    }
    for (size_t i = 0; i < width * width; i++)
    {
        (*destination)->grid[i] = (channel *)malloc(sizeof(channel));
        if (!(*destination)->grid[i])
        {
            return MALLOC_FAIL;
        }
    }
    return SUCCESS;
}

RESULT_CODE close_all_pipes(channel_grid *grid)
{
    RESULT_CODE result;
    for (size_t i = 0; i < grid->width; i++)
    {
        for (size_t j = 0; j < grid->width; j++)
        {
            if (i == j)
            {
                continue;
            }
            channel *local_channel;
            result = get_from_grid(grid, i, j, &local_channel);
            if (result)
            {
                return result;
            }
            close(local_channel->read);
            close(local_channel->write);
        }
    }
    return SUCCESS;
}

RESULT_CODE close_unused_pipes(grid_id_pair *pair)
{
    channel_grid *grid = pair->grid;
    local_id id = pair->id;
    RESULT_CODE result;

    for (size_t i = 0; i < grid->width; i++)
    {
        for (size_t j = 0; j < grid->width; j++)
        {
            if (i == j || (local_id)i == id)
            {
                continue;
            }
            channel *local_channel;
            result = get_from_grid(grid, i, j, &local_channel);
            if (result)
            {
                return result;
            }
            close(local_channel->read);
            close(local_channel->write);
        }
    }

    return SUCCESS;
}

void free_grid(channel_grid **grid)
{
    for (size_t i = 0; i < (*grid)->width * (*grid)->width; i++)
    {
        free((*grid)->grid[i]);
    }
    free((*grid)->grid);
    free(*grid);
    *grid = NULL;
}

#define BUFFER_SIZE 1024

RESULT_CODE fill_grid(channel_grid *grid)
{
    int pipe_fd[2];
    RESULT_CODE result = SUCCESS;
    char buffer[BUFFER_SIZE];
    int chars_written;
    for (size_t i = 0; i < grid->width; i++)
    {
        for (size_t j = i + 1; j < grid->width; j++)
        {
            channel *first;
            channel *second;
            result = get_from_grid(grid, i, j, &first);
            if (result)
            {
                return result;
            }
            result = get_from_grid(grid, j, i, &second);
            if (result)
            {
                return result;
            }
            if (pipe(pipe_fd))
            {
                return PIPE_FAIL;
            }
            first->read = pipe_fd[0];
            second->write = pipe_fd[1];
            if (pipe(pipe_fd))
            {
                return PIPE_FAIL;
            }
            second->read = pipe_fd[0];
            first->write = pipe_fd[1];

            chars_written = sprintf(buffer,
                                    "channel<%zu;%zu> {\n"
                                    "\t<%zu>.read = %d;\n"
                                    "\t<%zu>.write = %d;\n"
                                    "\t<%zu>.read = %d;\n"
                                    "\t<%zu>.write = %d;\n}\n",
                                    i, j,
                                    i, first->read,
                                    i, first->write,
                                    j, second->read,
                                    j, second->write);
            if (chars_written < 0)
            {
                return BUFFER_WRITE_FAIL;
            }
            result = pipe_log(buffer, chars_written);
            if (result)
            {
                return result;
            }
        }
    }
    return SUCCESS;
}

RESULT_CODE get_from_grid(channel_grid *grid, size_t i, size_t j, channel **dest)
{
    if (i >= grid->width || j >= grid->width)
    {
        return ADDRESSATION_FAIL;
    }
    *dest = grid->grid[i * grid->width + j];

    return SUCCESS;
}

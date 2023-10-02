#ifndef __IFMO_DISTRIBUTED_CLASS_GRID_H
#define __IFMO_DISTRIBUTED_CLASS_GRID_H

#include "logger.h"
#include "ipc.h"

struct channel
{
    int read;
    int write;
};

typedef struct channel channel;

struct channel_grid
{
    channel **grid;
    size_t width;
};

typedef struct channel_grid channel_grid;

struct grid_id_pair
{
    channel_grid *grid;
    local_id id;
};

typedef struct grid_id_pair grid_id_pair;

RESULT_CODE allocate_grid(size_t width, channel_grid **destination);
RESULT_CODE close_all_pipes(channel_grid *grid);
RESULT_CODE close_unused_pipes(grid_id_pair *pair);
void free_grid(channel_grid **grid);

RESULT_CODE fill_grid(channel_grid *grid);

RESULT_CODE get_from_grid(channel_grid *grid, size_t i, size_t j, channel **dest);

#endif

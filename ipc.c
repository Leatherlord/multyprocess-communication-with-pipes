#include "ipc.h"
#include "grid.h"

int send(void *self, local_id dst, const Message *msg)
{
    grid_id_pair *pair = (grid_id_pair *)self;
    channel_grid *grid = pair->grid;
    local_id id = pair->id;
    RESULT_CODE result;

    if (id == dst)
    {
        return (int)ADDRESSATION_FAIL;
    }

    channel *local_channel;
    result = get_from_grid(grid, (size_t)id, (size_t)dst, &local_channel);
    if (result)
    {
        return (int)result;
    }

    int acc = 0;
    int bytes_written;
    int amount = sizeof(MessageHeader) + msg->s_header.s_payload_len;
    while (acc < amount)
    {
        bytes_written = write(local_channel->write, msg + acc, amount - acc);
        if (bytes_written == -1)
        {
            return (int)PIPE_WRITE_FAIL;
        }
        acc += bytes_written;
    }

    return (int)SUCCESS;
}

int send_multicast(void *self, const Message *msg)
{
    grid_id_pair *pair = (grid_id_pair *)self;
    channel_grid *grid = pair->grid;
    local_id id = pair->id;
    RESULT_CODE result;

    for (size_t i = 0; i < grid->width; i++)
    {
        if ((local_id)i == id)
        {
            continue;
        }
        result = (RESULT_CODE)send(pair, (local_id)i, msg);
        if (result)
        {
            return (int)result;
        }
    }

    return (int)SUCCESS;
}

int receive(void *self, local_id from, Message *msg)
{
    grid_id_pair *pair = (grid_id_pair *)self;
    channel_grid *grid = pair->grid;
    local_id id = pair->id;
    RESULT_CODE result;

    channel *local_channel;
    result = get_from_grid(grid, (size_t)id, (size_t)from, &local_channel);
    if (result)
    {
        return (int)result;
    }

    int acc = 0;
    int bytes_read;
    int amount = sizeof(MessageHeader);
    bytes_read = read(local_channel->read, &(msg->s_header) + acc, amount - acc);
    if (bytes_read == 0)
    {
        return (int)NO_MSG_READY;
    }
    if (bytes_read == -1)
    {
        return (int)PIPE_READ_FAIL;
    }
    acc += bytes_read;
    while (acc < amount)
    {
        bytes_read = read(local_channel->read, &(msg->s_header) + acc, amount - acc);
        if (bytes_read == -1)
        {
            return (int)PIPE_READ_FAIL;
        }
        acc += bytes_read;
    }

    acc = 0;
    amount = msg->s_header.s_payload_len;
    while (acc < amount)
    {
        bytes_read = read(local_channel->read, msg->s_payload + acc, amount - acc);
        if (bytes_read == -1)
        {
            return (int)PIPE_READ_FAIL;
        }
        acc += bytes_read;
    }

    return (int)SUCCESS;
}

int receive_any(void *self, Message *msg)
{
    grid_id_pair *pair = (grid_id_pair *)self;
    channel_grid *grid = pair->grid;
    local_id id = pair->id;
    RESULT_CODE result;
    bool received = false;

    while (!received)
    {
        for (size_t i = 0; i < grid->width; i++)
        {
            if ((local_id)i == id)
            {
                continue;
            }
            result = (RESULT_CODE)receive(pair, (local_id)i, msg);
            if (result == NO_MSG_READY)
            {
                continue;
            }
            if (result)
            {
                return (int)result;
            }
        }
    }

    return (int)SUCCESS;
}

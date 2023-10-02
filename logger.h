#ifndef __IFMO_DISTRIBUTED_CLASS_LOGGER_H
#define __IFMO_DISTRIBUTED_CLASS_LOGGER_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "util.h"

RESULT_CODE open_files();
RESULT_CODE close_files();
RESULT_CODE event_log(char *buffer, size_t count);
RESULT_CODE pipe_log(char *buffer, size_t count);

#endif

#ifndef CLI_H
#define CLI_H

#include "container.h"

int parse_args(
    int argc,
    char *argv[],
    ContainerConfig *config
);

#endif
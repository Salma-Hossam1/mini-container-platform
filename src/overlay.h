#ifndef OVERLAY_H
#define OVERLAY_H

int create_overlay_dirs(
    const char *container_name
);

int fix_overlay_permissions(
    const char *container_name
);

int mount_overlay(
    const char *container_name
);

int unmount_overlay(
    const char *container_name
);

void get_merged_path(
    const char *container_name,
    char *buffer,
    int size
);

#endif // OVERLAY_H
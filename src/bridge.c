#include "bridge.h"

int ensure_bridge_exists(void)
{
    // Check if the bridge already exists
    if (system("ip link show mini0 > /dev/null 2>&1") != 0)
    {
        // Create the bridge
        if (system("ip link add name mini0 type bridge") != 0)
        {
            return -1;
        }
        // Assign IP address to the bridge
        if (system("ip addr add 10.0.0.1/24 dev mini0") != 0)
        {
            return -1;
        }
        // Bring the bridge up
        if (system("ip link set mini0 up") != 0)
        {
            return -1;
        }
    }

    return 0;
}

int attach_interface_to_bridge(const char *host_if)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip link set %s master mini0", host_if);
    if (system(cmd) != 0)
    {
        return -1;
    }
    return 0;
}


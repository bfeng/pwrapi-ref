#include "pwr_rnet_remote.h"
#include "pwr_dev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

static plugin_devops_t devops = {
    .open  = pwr_rnet_remote_open,
    .close = pwr_rnet_remote_close
};

plugin_devops_t *pwr_rnet_remote_init(const char *initstr)
{
}

int pwr_rnet_remote_final(plugin_devops_t *dev)
{
}

pwr_fd_t pwr_rnet_remote_open(plugin_devops_t *dev, const char *openstr)
{
}

int pwr_rnet_remote_close(pwr_fd_t fd)
{
}

static plugin_dev_t dev = {
    .init  = pwr_rnet_remote_init,
    .final = pwr_rnet_remote_final
};

plugin_dev_t *getDev()
{
    return &dev;
}

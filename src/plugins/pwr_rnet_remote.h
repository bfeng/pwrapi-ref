/*
 * Copyright 2014-2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 *
 * This file is part of the Power API Prototype software package. For license
 * information, see the LICENSE file in the top level directory of the
 * distribution.
 */

#ifndef PWR_RNET_REMOTE_DEV_H
#define PWR_RNET_REMOTE_DEV_H

#include "pwrdev.h"
#include "pwr_config.h"
#include "debug.h"

#define DBGP(X, ... ) DBG3( DBG_PLUGGIN, "Plugin", X, ##__VA_ARGS__ )

#ifdef __cplusplus
extern "C" {
#endif

    plugin_devops_t *pwr_rnet_remote_init(const char *initstr);
    int pwr_rnet_remote_final(plugin_devops_t *dev);

    pwr_fd_t pwr_rnet_remote_open(plugin_devops_t *dev, const char *openstr);
    int pwr_rnet_remote_close(pwr_fd_t fd);

#ifdef __cplusplus
}
#endif

#endif

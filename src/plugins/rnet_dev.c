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
#include <sys/timeb.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

#include "pwr_dev.h"
#include "util.h"

typedef struct {
    char config[100];
} rnetDevInfo_t;

#define BUFFER_LEN 10

typedef struct {
    double values[BUFFER_LEN];
    PWR_Time timeStamps[BUFFER_LEN];
} buffer_t;

typedef struct {
    buffer_t buffers[PWR_NUM_ATTR_NAMES];
} rnetFdInfo_t;

typedef struct {
    int perf_fd;
    long long value;
    double scale;
    struct perf_event_attr *attr_ptr;
} domainInfo_t;

static double getTime() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    double value;
    value = tv.tv_sec * 1000000000;
    value += tv.tv_usec * 1000;
    return value;
}

#define TYP_PTH "/sys/bus/event_source/devices/power/type"
#define EVT_FMT "/sys/bus/event_source/devices/power/events/%s"
#define SCL_FMT "/sys/bus/event_source/devices/power/events/%s.scale"
#define UNT_FMT "/sys/bus/event_source/devices/power/events/%s.unit"

static int rapl_perf_init(const char *domain, domainInfo_t *domain_info)
{

    DBGP("\n");
    FILE *fff;
    char filename[128];
    int type = 0;
    int config = 0;
    double scale = 0.0;

    fff = fopen(TYP_PTH, "r");
    if(fff != NULL)
    {
        if(fscanf(fff, "%d", &type) > 0)
        {
            DBGP("Found type=%d\n", type);
        }
        fclose(fff);
    }
    else
        goto ret_failure;

    sprintf(filename, EVT_FMT, domain);
    fff = fopen(filename, "r");
    if(fff != NULL)
    {
        if(fscanf(fff, "event=%x", &config) > 0)
        {
            DBGP("Found config=%d\n", config);
        }
        fclose(fff);
    }
    else
    {
        DBGP("%s not found\n", domain);
        goto ret_failure;
    }

    sprintf(filename, SCL_FMT, domain);
    fff = fopen(filename, "r");
    if(fff != NULL)
    {
        if(fscanf(fff, "%lf", &scale) > 0)
        {
            DBGP("Found scale=%f\n", scale);
        }
        fclose(fff);
    }
    else
        goto ret_failure;

    /*
       sprintf(filename, UNT_FMT, this->m_DomainName);
       fff = fopen(filename, "r");
       if(fff != NULL)
       {
       if(fscanf(fff, "%s", this->m_Units) > 0)
       {
       cout << "Found units=" << this->m_Units << endl;
       }
       fclose(fff);
       }
       else
       goto ret_failure;
       */

    goto ret_success;

ret_success:
    domain_info->attr_ptr->type = type;
    domain_info->attr_ptr->config = config;
    domain_info->scale = scale;
    return PWR_RET_SUCCESS;
ret_failure:
    return PWR_RET_FAILURE;
}

static int perf_event_open(
        struct perf_event_attr *hw_event_uptr,
        pid_t pid,
        int cpu,
        int group_fd,
        unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd, flags);
}

static int rapl_perf_open(domainInfo_t *domain_info)
{
    int core = 0;
    domain_info->perf_fd = 0;
    if(domain_info->attr_ptr == NULL)
    {
        DBGP("ATTR is NULL\n");
        goto ret_failure;
    }
    DBGP("Error before opening: %s\n", strerror(errno));
    domain_info->perf_fd = perf_event_open(domain_info->attr_ptr, -1, core, -1, 0);
    domain_info->perf_fd = perf_event_open(domain_info->attr_ptr, -1, core, -1, 0);
    /**perf_fd_ptr = syscall(__NR_perf_event_open, attr_ptr, -1, 0, -1, 0);*/
    DBGP("perf_fd=%d\n", domain_info->perf_fd);
    if(domain_info->perf_fd < 0)
    {
        if(errno == EACCES)
        {
            DBGP("Permission denied\n");
        }
        else
        {
            DBGP("Error opening: %s\n", strerror(errno));
        }
        goto ret_failure;
    }
    goto ret_success;
ret_success:
    return PWR_RET_SUCCESS;
ret_failure:
    return PWR_RET_FAILURE;
}

static int rapl_perf_read(domainInfo_t *domain_info)
{
    sleep(2);
    long long value = 0L;
    if(domain_info->perf_fd > 0)
    {
        if(read(domain_info->perf_fd, &value, 8) > 0)
        {
            domain_info->value = value;
            DBGP("Value: %lld\n", value);
        }
        else
        {
            DBGP("Error: %s\n", strerror(errno));
            return PWR_RET_FAILURE;
        }
    }
    return PWR_RET_SUCCESS;
}

static int rapl_perf_close(domainInfo_t *domain_info)
{
    if(domain_info->perf_fd)
    {
        close(domain_info->perf_fd);
        domain_info->perf_fd = -1;
    }
}

static int rapl_perf_free(domainInfo_t *domain_info)
{
    free(domain_info->attr_ptr);
    free(domain_info);
}

static double rapl_perf_read_domain(const char *domain)
{
    double result;
    domainInfo_t *domain_info = (domainInfo_t *) malloc(sizeof(domainInfo_t));
    domain_info->attr_ptr = (struct perf_event_attr *) malloc(sizeof(struct perf_event_attr));

    rapl_perf_init(domain, domain_info);
    rapl_perf_open(domain_info);

    rapl_perf_read(domain_info);
    result = (double)domain_info->value * domain_info->scale;

    rapl_perf_close(domain_info);
    rapl_perf_free(domain_info);

    return result;
}

static pwr_fd_t rnet_dev_open( plugin_devops_t* ops, const char *openstr )
{
    rnetFdInfo_t *tmp = malloc( 2*sizeof( rnetFdInfo_t ) );

    tmp->buffers[PWR_ATTR_POWER].values[0] = 10.1234;
    tmp->buffers[PWR_ATTR_ENERGY].values[0] = rapl_perf_read_domain("energy-pkg");
    DBGP("`%s` ptr=%p\n",openstr,tmp);

    return tmp;
}

static int rnet_dev_close( pwr_fd_t fd )
{
    DBGP("\n");
    free( fd );

    return 0;
}

static int rnet_dev_read( pwr_fd_t fd, PWR_AttrName type, void* ptr, unsigned int len, PWR_Time* ts )
{

    *(double*)ptr = ((rnetFdInfo_t*) fd)->buffers[type].values[0];

    DBGP("%p type=%s %f\n", fd, attrNameToString(type),*(double*)ptr);

    if ( ts ) {
        *ts = getTime();
    }

    return PWR_RET_SUCCESS;
}

static int rnet_dev_write( pwr_fd_t fd, PWR_AttrName type, void* ptr, unsigned int len )
{
    DBGP("type=%s %f\n",attrNameToString(type), *(double*)ptr);

    ((rnetFdInfo_t*) fd)->buffers[type].values[0] = *(double*)ptr;
    return PWR_RET_SUCCESS;
}

static int rnet_dev_readv( pwr_fd_t fd, unsigned int arraysize, const PWR_AttrName attrs[], void* buf,
        PWR_Time ts[], int status[] )
{
    int i;
    for ( i = 0; i < arraysize; i++ ) {

        ((double*)buf)[i] = ((rnetFdInfo_t*) fd)->buffers[attrs[i]].values[0];

        DBGP("type=%s %f\n",attrNameToString(attrs[i]), ((double*)buf)[i]);

        ts[i] = getTime();

        status[i] = PWR_RET_SUCCESS;
    }
    return PWR_RET_SUCCESS;
}

static int rnet_dev_writev( pwr_fd_t fd, unsigned int arraysize, const PWR_AttrName attrs[], void* buf, int status[] )
{
    int i;
    DBGP("num attributes %d\n",arraysize);
    for ( i = 0; i < arraysize; i++ ) {
        DBGP("type=%s %f\n",attrNameToString(attrs[i]), ((double*)buf)[i]);

        ((rnetFdInfo_t*) fd)->buffers[attrs[i]].values[0] = ((double*)buf)[i];

        status[i] = PWR_RET_SUCCESS;
    }
    return PWR_RET_SUCCESS;
}

static int rnet_dev_time( pwr_fd_t fd, PWR_Time *timestamp )
{
    DBGP("\n");

    return 0;
}

static int rnet_dev_clear( pwr_fd_t fd )
{
    DBGP("\n");

    return 0;
}

static int rnet_dev_log_start( pwr_fd_t fd, PWR_AttrName name )
{
    buffer_t* ptr = &((rnetFdInfo_t*) fd)->buffers[name];
    DBGP("\n");
    ptr->timeStamps[0] = getTime();
    return PWR_RET_SUCCESS;
}

static int rnet_dev_log_stop( pwr_fd_t fd, PWR_AttrName name )
{
    return PWR_RET_SUCCESS;
}

static int rnet_dev_get_samples( pwr_fd_t fd, PWR_AttrName name,
        PWR_Time* ts, double period, unsigned int* nSamples, void* buf )

{
    DBGP("period=%f samples=%d\n",period,*nSamples);
    struct timeb tp;
    ftime( &tp);
    srand((unsigned) tp.millitm );
    int i;

    struct timeval tv;
    gettimeofday(&tv,NULL);

    srand(  tv.tv_usec );
    for ( i = 0; i < *nSamples; i++ ) {
        ((double*)buf)[i] = 100 + (float)rand()/(float)( RAND_MAX/2.0);
        DBGP("%f\n",((double*)buf)[i]);
    }
    *ts = getTime() - ( (*nSamples * period) * 1000000000);
    DBGP("ts=%jd\n",*ts);
    return PWR_RET_SUCCESS;
}

static plugin_devops_t devOps = {
    .open   = rnet_dev_open,
    .close  = rnet_dev_close,
    .read   = rnet_dev_read,
    .write  = rnet_dev_write,
    .readv  = rnet_dev_readv,
    .writev = rnet_dev_writev,
    .time   = rnet_dev_time,
    .clear  = rnet_dev_clear,
    .log_start = rnet_dev_log_start,
    .log_stop = rnet_dev_log_stop,
    .get_samples = rnet_dev_get_samples,
};

static plugin_devops_t* rnet_dev_init( const char *initstr )
{
    DBGP("RNet driver init\n");
    plugin_devops_t* ops = malloc(sizeof(*ops));
    *ops = devOps;
    ops->private_data = malloc( sizeof( rnetDevInfo_t ) );
    return ops;
}

static int rnet_dev_final( plugin_devops_t *ops )
{
    DBGP("\n");
    free( ops->private_data );
    free( ops );
    return 0;
}

static plugin_dev_t dev = {
    .init   = rnet_dev_init,
    .final  = rnet_dev_final,
};

plugin_dev_t* getDev() {
    return &dev;
}

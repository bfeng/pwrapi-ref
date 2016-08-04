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

#include "pwr_dev.h"
#include "util.h"

typedef struct {
	char config[100];
} dummyDevInfo_t;

#define BUFFER_LEN 10

typedef struct {
	double values[BUFFER_LEN];
	PWR_Time timeStamps[BUFFER_LEN];
} buffer_t;

typedef struct {
	buffer_t buffers[PWR_NUM_ATTR_NAMES];
} dummyFdInfo_t;

static double getTime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	double value;
	value = tv.tv_sec * 1000000000;
	value += tv.tv_usec * 1000;
	return value;
}

static pwr_fd_t dummy_dev_open( plugin_devops_t* ops, const char *openstr )
{
	dummyFdInfo_t *tmp = malloc( 2*sizeof( dummyFdInfo_t ) );
	tmp->buffers[PWR_ATTR_POWER].values[0] = 10.1234;
	tmp->buffers[PWR_ATTR_ENERGY].values[0] = 100000000;
	DBGP("`%s` ptr=%p\n",openstr,tmp);
	return tmp;
}

static int dummy_dev_close( pwr_fd_t fd )
{
	DBGP("\n");
	free( fd );

	return 0;
}

static int dummy_dev_read( pwr_fd_t fd, PWR_AttrName type, void* ptr, unsigned int len, PWR_Time* ts )
{

	*(double*)ptr = ((dummyFdInfo_t*) fd)->buffers[type].values[0];

	DBGP("%p type=%s %f\n", fd, attrNameToString(type),*(double*)ptr);

	if ( ts ) {
		*ts = getTime();
	}

	return PWR_RET_SUCCESS;
}

static int dummy_dev_write( pwr_fd_t fd, PWR_AttrName type, void* ptr, unsigned int len )
{
	DBGP("type=%s %f\n",attrNameToString(type), *(double*)ptr);

	((dummyFdInfo_t*) fd)->buffers[type].values[0] = *(double*)ptr;
	return PWR_RET_SUCCESS;
}

static int dummy_dev_readv( pwr_fd_t fd, unsigned int arraysize, const PWR_AttrName attrs[], void* buf,
		PWR_Time ts[], int status[] )
{
	int i;
	for ( i = 0; i < arraysize; i++ ) {

		((double*)buf)[i] = ((dummyFdInfo_t*) fd)->buffers[attrs[i]].values[0];

		DBGP("type=%s %f\n",attrNameToString(attrs[i]), ((double*)buf)[i]);

		ts[i] = getTime();

		status[i] = PWR_RET_SUCCESS;
	}
	return PWR_RET_SUCCESS;
}

static int dummy_dev_writev( pwr_fd_t fd, unsigned int arraysize, const PWR_AttrName attrs[], void* buf, int status[] )
{
	int i;
	DBGP("num attributes %d\n",arraysize);
	for ( i = 0; i < arraysize; i++ ) {
		DBGP("type=%s %f\n",attrNameToString(attrs[i]), ((double*)buf)[i]);

		((dummyFdInfo_t*) fd)->buffers[attrs[i]].values[0] = ((double*)buf)[i];

		status[i] = PWR_RET_SUCCESS;
	}
	return PWR_RET_SUCCESS;
}

static int dummy_dev_time( pwr_fd_t fd, PWR_Time *timestamp )
{
	DBGP("\n");

	return 0;
}

static int dummy_dev_clear( pwr_fd_t fd )
{
	DBGP("\n");

	return 0;
}

static int dummy_dev_log_start( pwr_fd_t fd, PWR_AttrName name )
{
	buffer_t* ptr = &((dummyFdInfo_t*) fd)->buffers[name];
	DBGP("\n");
	ptr->timeStamps[0] = getTime();
	return PWR_RET_SUCCESS;
}

static int dummy_dev_log_stop( pwr_fd_t fd, PWR_AttrName name )
{
	return PWR_RET_SUCCESS;
}

static int dummy_dev_get_samples( pwr_fd_t fd, PWR_AttrName name,
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
	DBGP("ts=%ld\n", *ts);
	return PWR_RET_SUCCESS;
}

static plugin_devops_t devOps = {
	.open   = dummy_dev_open,
	.close  = dummy_dev_close,
	.read   = dummy_dev_read,
	.write  = dummy_dev_write,
	.readv  = dummy_dev_readv,
	.writev = dummy_dev_writev,
	.time   = dummy_dev_time,
	.clear  = dummy_dev_clear,
	.log_start = dummy_dev_log_start,
	.log_stop = dummy_dev_log_stop,
	.get_samples = dummy_dev_get_samples,
};

static plugin_devops_t* dummy_dev_init( const char *initstr )
{
	plugin_devops_t* ops = malloc(sizeof(*ops));
	*ops = devOps;
	ops->private_data = malloc( sizeof( dummyDevInfo_t ) );
	return ops;
}

static int dummy_dev_final( plugin_devops_t *ops )
{
	DBGP("\n");
	free( ops->private_data );
	free( ops );
	return 0;
}

static plugin_dev_t dev = {
	.init   = dummy_dev_init,
	.final  = dummy_dev_final,
};

plugin_dev_t* getDev() {
	return &dev;
}

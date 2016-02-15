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

#include "pwr.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_ATTR(X) (sizeof(X)/sizeof(PWR_AttrName))

int section_4_6_test( void )
{
    int rc, i;
    PWR_Cntxt cntxt;
    PWR_Obj self;
    PWR_Stat stat_avg, stat_max;
    double val;
    PWR_TimePeriod tp;
    PWR_AttrName attrs[] = { PWR_ATTR_POWER, PWR_ATTR_ENERGY };
    double vals[NUM_ATTR(attrs)];
    PWR_TimePeriod tps[NUM_ATTR(attrs)];

    printf( "\tPWR_CntxtInit - application context\n" );
	rc = PWR_CntxtInit( PWR_CNTXT_DEFAULT, PWR_ROLE_APP, "Application", &cntxt );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: initialization of PowerAPI context failed\n" );
        return -1;
    }

    printf( "\tPWR_CntxtGetEntryPoint\n" );
	rc = PWR_CntxtGetEntryPoint( cntxt, &self );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: getting self from PowerAPI context failed\n" );
        return -1;
    }

#if 0
    printf( "\tPWR_CreateStat - PWR_ATTR_STAT_AVG of PWR_ATTR_POWER\n" );
    rc = PWR_ObjCreateStat( self, PWR_ATTR_POWER, PWR_ATTR_STAT_AVG, stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: creating stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatStart - average power\n" );
    rc = PWR_StatStart( stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: starting stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatStop - average power\n" );
    rc = PWR_StatStop( stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: stoping stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatGetValue - average power\n" );
    rc = PWR_StatGetValue( stat_avg, &val, &tp );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: stoping stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatClear - average power\n" );
    rc = PWR_StatClear( stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: clearing stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatGetReduce - PWR_ATTR_STAT_MIN of average power\n" );
    rc = PWR_StatGetReduce( stat, PWR_ATTR_STAT_MIN, &i, vals, tps );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: stoping stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatClear - maximum power and energy\n" );
    rc = PWR_StatClear( stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: clearing stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatDestroy\n" );
    rc = PWR_StatDestroy( stat_avg );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: destruction of stat failed\n" );
        return -1;
    }

    printf( "\tPWR_CreateStat - PWR_ATTR_STAT_MAX of PWR_ATTR_POWER, PWR_ATTR_ENERGY\n" );
    rc = PWR_ObjCreateStat( self, PWR_ATTR_POWER, PWR_ATTR_STAT_AVG, stat_max );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: creating stat for objects failed\n" );
        return -1;
    }

    printf( "\tPWR_StatStart - maximum power, energy\n" );
    rc = PWR_StatStart( stat_max );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: starting stat for objects failed\n" );
        return -1;
    }

    printf( "\tPWR_StatStop - maximum power, energy\n" );
    rc = PWR_StatStop( stat_max );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: stoping stat for objects failed\n" );
        return -1;
    }

    printf( "\tPWR_StatGetValues - maximum power, energy\n" );
    rc = PWR_StatGetValues( stat_max, vals, tps );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: stoping stat for objects failed\n" );
        return -1;
    }

    printf( "\tPWR_StatClear - maximum power and energy\n" );
    rc = PWR_StatClear( stat_max );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: clearing stat for object failed\n" );
        return -1;
    }

    printf( "\tPWR_StatDestroy\n" );
    rc = PWR_StatDestroy( stat_max );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: destruction of stat failed\n" );
        return -1;
    }
#endif

    printf( "\tPWR_CntxtDestroy - application context\n" );
    rc = PWR_CntxtDestroy( cntxt );
    if( rc != PWR_RET_SUCCESS ) {
        printf( "\t\tError: destruction of PowerAPI context failed\n" );
        return -1;
    }

    return 0;
}
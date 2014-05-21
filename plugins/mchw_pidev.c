#include "mchw_pidev.h"
#include "piapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

static int pidev_verbose = 1;

typedef struct {
    void *cntx;
    piapi_port_t port;
} mchw_pidev_t;
#define MCHW_PIDEV(X) ((mchw_pidev_t *)(X))

static piapi_sample_t pidev_counter;
static int pidev_reading;

static void pidev_callback( piapi_sample_t *sample )
{
    if( sample->total && sample->number == sample->total )
        pidev_reading = 0;

    pidev_counter = *sample;
}

static int pidev_parse( char *initstr, unsigned int *saddr, unsigned int *sport, unsigned int *port )
{
    int shift = 24;
    char *token;

    if( pidev_verbose )
        printf( "Info: received initialization string %s\n", initstr );

    *saddr = 0;
    while( shift >= 0 ) {
        if( (token = strtok( (shift!=24) ? NULL : initstr, (shift!=0) ? "." : ":" )) == 0x0 ) {
            printf( "Error: invalid server IP address in initialization string %s\n", initstr );
            return -1;
        }
        *saddr |= ( atoi(token) << shift );
        shift -= 8;
        printf( "Info: extracted initialization string (SADDR=%08x, SPORT=%u)\n", *saddr, *sport );
    }

    if( (token = strtok( NULL, ":" )) == 0x0 ) {
        printf( "Error: missing server port separator in initialization string %s\n", initstr );
        return -1;
    }
    *sport = atoi(token);

    if( (token = strtok( NULL, ":" )) == 0x0 ) {
        printf( "Error: missing sensor port separator in initialization string %s\n", initstr );
        return -1;
    }
    *port = atoi(token);

    if( pidev_verbose )
        printf( "Info: extracted initialization string (SADDR=%08x, SPORT=%u, PORT=%u)\n", *saddr, *sport, *port );

    return 0;
}

int mchw_pidev_open( pwr_dev_t *dev, char *initstr )
{
    unsigned int saddr = 0, sport = 0, port = 0;

    *dev = malloc( sizeof(mchw_pidev_t) );
    bzero( *dev, sizeof(mchw_pidev_t) );

    if( initstr == 0x0 || pidev_parse(initstr, &saddr, &sport, &port) < 0 ) {
        printf( "Error: invalid monitor and control hardware initialization string\n" );
        return -1;
    }

    if( piapi_init( &(MCHW_PIDEV(*dev)->cntx), PIAPI_MODE_PROXY, pidev_callback, saddr, sport ) < 0 ) {
        printf( "Error: powerinsight hardware initialization failed\n" );
        return -1;
    }

    return 0;
}

int mchw_pidev_close( pwr_dev_t *dev )
{
    if( piapi_destroy( &(MCHW_PIDEV(*dev)->cntx) ) < 0 ) {
        printf( "Error: powerinsight hardware finalization failed\n" );
        return -1;
    }

    free( *dev );
    *dev = 0x0;

    return 0;
}

int mchw_pidev_read( pwr_dev_t dev, unsigned int arraysize,
	PWR_AttrType type[], float reading[], unsigned long long *timestamp )
{
    unsigned int i;

    while( pidev_reading ) sched_yield();
    pidev_reading = 1;
    if( piapi_counter( MCHW_PIDEV(dev)->cntx, MCHW_PIDEV(dev)->port ) < 0 ) {
        printf( "Error: powerinsight hardware read failed\n" );
        return -1;
    }
    while( pidev_reading ) sched_yield();

    for( i = 0; i < arraysize; i++ ) {
        switch( type[i] ) {
            case PWR_ATTR_VOLTAGE:
                reading[i] = pidev_counter.raw.volts;
                break;
            case PWR_ATTR_CURRENT:
                reading[i] = pidev_counter.raw.amps;
                break;
            case PWR_ATTR_POWER:
                reading[i] = pidev_counter.raw.watts;
                break;
            case PWR_ATTR_MIN_POWER:
                reading[i] = pidev_counter.min.watts;
                break;
            case PWR_ATTR_MAX_POWER:
                reading[i] = pidev_counter.max.watts;
                break;
            case PWR_ATTR_ENERGY:
                reading[i] = pidev_counter.energy;
                break;
            default:
                printf( "Warning: unknown MCHW reading type (%u) requested at position %u\n", type[i], i );
                break;
        }
    }
    *timestamp = pidev_counter.time_sec*1000000000ULL + 
                 pidev_counter.time_usec*1000;

    return 0;
}

int mchw_pidev_write( pwr_dev_t dev, unsigned int arraysize,
	PWR_AttrType type[], float setting[], unsigned long long *timestamp )
{
    return 0;
}

int mchw_pidev_time( pwr_dev_t dev, unsigned long long *time )
{
    return mchw_pidev_read( dev, 0, 0x0, 0x0, time );
}


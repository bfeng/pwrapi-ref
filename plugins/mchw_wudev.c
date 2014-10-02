#include "mchw_wudev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

static int wudev_verbose = 0;

typedef struct {
    int fd;
} mchw_wudev_t;
#define MCHW_WUDEV(X) ((mchw_wudev_t *)(X))

typedef struct {
    mchw_wudev_t *dev;
} mchw_wufd_t;
#define MCHW_WUFD(X) ((mchw_wufd_t *)(X))

static int wudev_parse( const char *initstr, char *port, int *baud )
{
    char *token;

    if( wudev_verbose )
        printf( "Info: received initialization string %s\n", initstr );

    if( (token = strtok( (char *)initstr, ":" )) == 0x0 ) {
        printf( "Error: missing serial port seperator in initialization string %s\n", initstr );
        return -1;
    }
    strcpy( port, token );

    if( (token = strtok( NULL, ":" )) == 0x0 ) {
        printf( "Error: missing serial baud seperator in initialization string %s\n", initstr );
        return -1;
    }
    *baud = atoi(token);

    if( wudev_verbose )
        printf( "Info: extracted initialization string (PORT=%s, BAUD=%d)\n", port, *baud);

    return 0;
}

static int wudev_open( const char *port, int baud )
{
    struct termios opts;
    int fd;
    speed_t rate;

    if( (fd=open( port, O_RDWR | O_NOCTTY | O_NDELAY)) == -1 )
        return -1;

    if( tcgetattr( fd, &opts ) < 0 )
        return -1;

    switch( baud ) {
        case 4800:
            rate = B4800;
            break;
        case 9600:
            rate = B9600;
            break;
        case 19200:
            rate = B19200;
            break;
        case 38400:
            rate = B38400;
            break;
        case 57600:
            rate = B57600;
            break;
        case 115200:
            rate = B115200;
            break;
        default:
            printf( "Error: unsupported baud rate of %d\n", baud );
            return -1;
    }

    cfsetispeed( &opts, rate );
    cfsetospeed( &opts, rate );

    opts.c_cflag &= ~PARENB;
    opts.c_cflag &= ~CSTOPB;
    opts.c_cflag &= ~CSIZE;
    opts.c_cflag |= CS8;

    opts.c_cflag &= ~CRTSCTS;
    opts.c_cflag |= CREAD | CLOCAL;
    opts.c_iflag &= ~(IXON | IXOFF | IXANY);

    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opts.c_oflag &= ~OPOST;

    opts.c_cc[VMIN] = 0;
    opts.c_cc[VTIME] = 20;

    if( tcsetattr( fd, TCSANOW, &opts) < 0 )
        return -1;

    return fd;
}

static int wudev_write( int fd, const char *str )
{
    int len = strlen( str );

    if( len > 0 && write( fd, str, len ) != len )
        return -1;

    return 0;
}

static int wudev_read( int fd, char *buf, char term )
{
    char c;
    int i = 0, n;

    do {
        n = read( fd, &c, 1 );

        if( n == -1 )
            return -1;

        if( n == 0 ) {
            usleep(5 * 1000);
            continue;
        }

        if( c != '\r' && c != '\n' )
            buf[i++] = c;
    } while( c != term );

    buf[i] = '\0';
    return 0;
}

static long long wudev_extract( const char *buf, int pos )
{
    char *token;
    int i;

    for( i = 0; i < pos; i++ )
        if( (token = strtok( i ? NULL : (char *)buf, "," )) == 0x0 )
            return 0;

    return atol(token);
}

pwr_dev_t mchw_wudev_init( const char *initstr )
{
    char port[256] = "";
    int baud = 0;

    pwr_dev_t *dev = malloc( sizeof(mchw_wudev_t) );
    bzero( dev, sizeof(mchw_wudev_t) );

    if( wudev_verbose )
        printf( "Info: initializing MCHW Wattsup device\n" );

    if( initstr == 0x0 || wudev_parse( initstr, port, &baud ) < 0 ) {
        printf( "Error: invalid monitor and control hardware initialization string\n" );
        return 0x0;
    }

    if( (MCHW_WUDEV(dev)->fd = wudev_open( port, baud )) < 0 ) {
        printf( "Error: wattsup hardware initialization failed\n" );
        return 0x0;
    }

    return dev;
}

int mchw_wudev_final( pwr_dev_t dev )
{
    if( wudev_verbose )
        printf( "Info: finalizing MCHW Wattsup device\n" );

    close( MCHW_WUDEV(dev)->fd );
    free( dev );

    return 0;
}

pwr_fd_t mchw_wudev_open( pwr_dev_t dev, const char *openstr )
{
    pwr_fd_t *fd = malloc( sizeof(mchw_wufd_t) );
    bzero( fd, sizeof(mchw_wufd_t) );

    if( wudev_verbose )
        printf( "Info: opening MCHW Wattsup descriptor\n" );

    MCHW_WUFD(fd)->dev = MCHW_WUDEV(dev);

    if( wudev_verbose )
        printf( "Info: opened file descriptor %d\n", MCHW_WUDEV(dev)->fd );

    return fd;
}

int mchw_wudev_close( pwr_fd_t fd )
{
    if( wudev_verbose )
        printf( "Info: closing MCHW Wattsup descriptor\n" );

    MCHW_WUFD(fd)->dev = 0x0;
    free( fd );

    return 0;
}

int mchw_wudev_read( pwr_fd_t fd, PWR_AttrName attr, void *value, unsigned int len, PWR_Time *timestamp )
{
    char buf[256] = "";

    if( wudev_verbose )
        printf( "Info: reading from MCHW Wattsup device\n" );
 
    if( wudev_write( MCHW_WUDEV(MCHW_WUFD(fd)->dev)->fd, "#L,W,3,E,0,1;" ) == -1 ) {
        printf( "Error: command write to Wattsup device failed\n" );
        return -1;
    }

    if( wudev_read( MCHW_WUDEV(MCHW_WUFD(fd)->dev)->fd, buf, ';' ) != 0 ) {
        printf( "Error: reading from Wattsup device failed\n" );
        return -1;
    }

    if( strlen( buf ) > 0 ) {
        if( wudev_verbose )
            printf( "Info: read buffer is %s\n", buf );

        switch( attr ) {
            case PWR_ATTR_VOLTAGE:
                *((double *)value) = wudev_extract( buf, 6 ) / 10;
                break;
            case PWR_ATTR_CURRENT:
                *((double *)value) = wudev_extract( buf, 7 ) / 1000;
                break;
            case PWR_ATTR_POWER:
                *((double *)value) = wudev_extract( buf, 5 ) / 10;
                break;
            default:
                printf( "Warning: unknown MCHW reading attr (%u) requested\n", attr );
                break;
        }
        *timestamp = wudev_extract( buf, 1 );
    }
    
    return 0;
}

int mchw_wudev_write( pwr_fd_t fd, PWR_AttrName attr, void *value, unsigned int len )
{
    if( wudev_verbose )
        printf( "Info: writing to MCHW Wattsup device\n" );

    switch( attr ) {
        default:
            printf( "Warning: unknown MCHW reading attr (%u)\n", attr );
            break;

        if( wudev_verbose )
            printf( "Info: setting of type %u with value %lf\n",
                    attr, *((double *)value) );
    }

    return 0;
}

int mchw_wudev_readv( pwr_fd_t fd, unsigned int arraysize,
    const PWR_AttrName attrs[], void *values, PWR_Time timestamp[], int status[] )
{
    unsigned int i;
    char buf[256] = "";

    if( wudev_verbose )
        printf( "Info: reading from MCHW Wattsup device\n" );
 
    if( wudev_write( MCHW_WUDEV(MCHW_WUFD(fd)->dev)->fd, "#L,W,3,E,0,1;" ) == -1 ) {
        printf( "Error: command write to Wattsup device failed\n" );
        return -1;
    }

    if( wudev_read( MCHW_WUDEV(MCHW_WUFD(fd)->dev)->fd, buf, ';' ) != 0 ) {
        printf( "Error: reading from Wattsup device failed\n" );
        return -1;
    }

    if( strlen( buf ) > 0 ) {
        if( wudev_verbose )
            printf( "Info: read buffer is %s\n", buf );

        for( i = 0; i < arraysize; i++ ) {
            switch( attrs[i] ) {
                case PWR_ATTR_VOLTAGE:
                    *((double *)values+i) = (double)wudev_extract( buf, 6 ) / 10; 
                    break;
                case PWR_ATTR_CURRENT:
                    *((double *)values+i) = (double)wudev_extract( buf, 7 ) / 1000; 
                    break;
                case PWR_ATTR_POWER:
                    *((double *)values+i) = (double)wudev_extract( buf, 5 ) / 10; 
                    break;
                default:
                    printf( "Warning: unknown MCHW reading attr (%u) requested at position %u\n", attrs[i], i );
                    break;
            }
            timestamp[i] = wudev_extract( buf, 1 );

            if( wudev_verbose )
                printf( "Info: reading of type %u at time %llu with value %lf\n",
                        attrs[i], *(unsigned long long *)timestamp[i], *((double *)(values+i)) );
        }
    }
    
    return 0;
}

int mchw_wudev_writev( pwr_fd_t fd, unsigned int arraysize,
    const PWR_AttrName attrs[], void *values, int status[] )
{
    unsigned int i;

    for( i = 0; i < arraysize; i++ ) {
        switch( attrs[i] ) {
            default:
                printf( "Warning: unknown MCHW writing attr (%u) requested at position %u\n", attrs[i], i );

        if( wudev_verbose )
            printf( "Info: setting of type %u with value %lf\n",
                    attrs[i], *((double *)(values+i)) );
        }
    }

    return 0;
}

int mchw_wudev_time( pwr_fd_t fd, PWR_Time *timestamp )
{
    double value;

    if( wudev_verbose )
        printf( "Info: getting time from MCHW Wattsup device\n" );

    return mchw_wudev_read( fd, PWR_ATTR_POWER, &value, sizeof(double), timestamp);
}

int mchw_wudev_clear( pwr_fd_t fd )
{
    if( wudev_verbose )
        printf( "Info: clearing MCHW Wattsup device\n" );

    return 0;
}

static plugin_dev_t dev = {
    .init   = mchw_wudev_init,
    .final  = mchw_wudev_final,
    .open   = mchw_wudev_open,
    .close  = mchw_wudev_close,
    .read   = mchw_wudev_read,
    .write  = mchw_wudev_write,
    .readv  = mchw_wudev_readv,
    .writev = mchw_wudev_writev,
    .time   = mchw_wudev_time,
    .clear  = mchw_wudev_clear
};

plugin_dev_t* getDev() {
    return &dev;
}
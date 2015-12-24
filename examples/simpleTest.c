/* 
 * Copyright 2014-2015 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 *
 * This file is part of the Power API Prototype software package. For license
 * information, see the LICENSE file in the top level directory of the
 * distribution.
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <pwr.h> 

char* myctime(const time_t *timep);

int main( int argc, char* argv[] )
{
    PWR_Grp     grp;
    PWR_Obj     self;
    PWR_Cntxt   cntxt;
    time_t      time;
    int         retval;
    double       value;
    PWR_Time ts;
	PWR_Status  status;

	assert( argc == 2 );	
    // Get a context
    cntxt = PWR_CntxtInit( PWR_CNTXT_DEFAULT, PWR_ROLE_APP, "App" );
    assert( PWR_NULL != cntxt   );

	PWR_Obj obj = PWR_CntxtGetObjByName( cntxt, argv[1] );
    assert( PWR_NULL != obj   );

#if 0
    self = PWR_CntxtGetEntryPoint( cntxt );
    assert( self );
    
    printf("I'm a `%s`\n", PWR_ObjGetTypeString( PWR_ObjGetType( self ) ) ); 

	
    PWR_Obj parent = PWR_ObjGetParent( self );
    assert( ! parent );

    PWR_Grp children = PWR_ObjGetChildren( self );
    assert( children );
#endif

    retval = PWR_ObjAttrGetValue( obj, PWR_ATTR_ENERGY, &value, &ts );
    assert( retval == PWR_RET_SUCCESS );

	printf("%f\n",value);
    return 0;
}
char* myctime(const time_t *timep)
{
    char* tmp = ctime(timep);
    tmp[ strlen(tmp) - 1 ] = 0; 
    return tmp;
}
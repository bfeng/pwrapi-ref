#!/bin/sh

#
# Copyright 2014 Sandia Corporation. Under the terms of Contract
# DE-AC04-94AL85000, there is a non-exclusive license for use of this work
# by or on behalf of the U.S. Government. Export of this program may require
# a license from the United States Government.
#
# This file is part of the Power API Prototype software package. For license
# information, see the LICENSE file in the top level directory of the
# distribution.
#


ROOT=/home/ddeboni/sw/pwrapi

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${ROOT}/lib:${ROOT}/lib/libpwr-plugins"
export DYLD_LIBRARY_PATH="${DYLD_LIBRARY_PATH}:${ROOT}/lib:${ROOT}/lib/libpwr-plugins"
export POWERAPI_CONFIG="${ROOT}/share/pwrapi/config/teller-platform.xml"
export POWERAPI_ROOT="teller.node40"

SAMPLES=100
FREQ=10

for ATTR in C P E
do
    echo "-----------------------------------------------"
    echo "Collecting ${ATTR} ${SAMPLES} at ${FREQ} Hz"
    echo "-----------------------------------------------"
    ${ROOT}/bin/pwrapi -s ${SAMPLES} -f ${FREQ} -a ${ATTR}
done

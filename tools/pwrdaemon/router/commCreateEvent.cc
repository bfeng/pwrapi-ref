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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <debug.h> 
#include "commCreateEvent.h" 
#include "router.h"
#include "rnetClient.h"

using namespace PWR_Router;

bool RtrCommCreateEvent::process(EventGenerator* _rtr, EventChannel* ec) {
    Router& rtr = *static_cast<Router*> (_rtr);
    Router::Client* client = rtr.getClient(ec);
    DBGX("id=%"PRIx64"\n", commID);

    client->addComm(commID, this);

    // for each object in a group
    for (unsigned int j = 0; j < members.size(); j++) {
        // for each source of this object 
        for (unsigned int i = 0; i < members[j].size(); i++) {
            DBGX("%s\n", members[j][i].c_str());

            CommCreateEvent* ev = new CommCreateEvent();
            ev->commID = commID;

            std::vector< ObjID > tmp;

            tmp.push_back(members[j][i]);
            ev->members.push_back(tmp);
            rtr.sendEvent(members[j][i], ev);
            delete ev;
        }
    }

    return false;
}

bool RNETRtrCommCreateEvent::process(EventGenerator* _rtr, EventChannel* ec) {
    Router& rtr = *static_cast<Router*> (_rtr);
    Router::Client* client = rtr.getClient(ec);
    DBGX("id=%"PRIx64"\n", commID);
    client->addComm(commID, this);

    // send to children
    if (!rtr.m_args.isLeaf) {
        DBGX("I'm not a leaf\n");
        DBGX("I'm going to forward all events to child routers.\n");
        RNET::POWERAPI::RNETClient new_client("bfeng-HP-Z240-Tower-Workstation", "25000");
        Event* ev = new Event();
        ev->type = RNETCommCreate;
        new_client.sendEvent(ev);
        delete ev;
    } else {
        DBGX("I'm a leaf\n");
    }

    return false;
}
/*
 * File:   rnetClient.h
 * Author: bfeng
 *
 * Created on July 15, 2016, 3:28 PM
 */

#ifndef RNETCLIENT_H
#define RNETCLIENT_H

#include <stdlib.h>
#include <vector>
#include <event.h>
#include <getopt.h>
#include "debug.h"
#include "eventChannel.h"
#include "events.h"
#include "router.h"

namespace RNET {
    namespace POWERAPI {

        struct Args {
            std::string routerHost;
            std::string routerPort;
        };

        class RNETClient : public EventGenerator {
            public:
                RNETClient(int, char* []);
                RNETClient(std::string, std::string);
                ~RNETClient();
                void sendEvent(Event *ev);
            private:
                CommunicatorStore * m_commStore;
                Args m_args;
                ChannelSelect * m_chanSelect;
                EventChannel * m_routerChannel;
        };
    }
}

#endif /* RNETCLIENT_H */


/*
 * File:   rnetClient.h
 * Author: bfeng
 *
 * Created on July 15, 2016, 3:28 PM
 */

#ifndef RNETCLIENT_H
#define RNETCLIENT_H

#include "debug.h"
#include "eventChannel.h"
#include "events.h"
#include "rnetCommunicator.h"
#include "router.h"
#include "routerCore.h"
#include <event.h>
#include <getopt.h>
#include <stdlib.h>
#include <vector>

namespace RNET {
    namespace POWERAPI {

        struct Args {
            std::string routerHost;
            std::string routerPort;
        };

        class RNETClient : public EventGenerator {
        public:
            RNETClient(int, char *[]);
            RNETClient(std::string, std::string);
            ~RNETClient();
            void sendEvent(Event *ev);
            void initWORLD();
            void initCommunicator(std::string key, PWR_Router::RouterID rtrs[], unsigned int n);
            void initCommunicator(std::string key, std::vector<PWR_Router::RouterID> rtrIDs);
            std::vector<PWR_Router::RouterID> lookupLeaves();
        private:
            CommunicatorStore *m_commStore;
            Args m_args;
            ChannelSelect *m_chanSelect;
            EventChannel *m_routerChannel;
        };
    }
}

#endif /* RNETCLIENT_H */

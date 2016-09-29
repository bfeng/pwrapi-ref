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
            RNETClient(std::string, unsigned int);
            ~RNETClient();
            void sendEvent(Event *ev);
            void initWORLD();
            void initCommunicator(std::string key, PWR_Router::RouterID rtrs[], unsigned int n);
            void initCommunicator(std::string key, std::vector<PWR_Router::RouterID> rtrIDs);

            void queryAndWait();

            void waitFor();

            /**
             * Check if the input routers are leaves
             * return leaf routers
             */
            std::vector<PWR_Router::RouterID> lookupLeaves(std::vector<PWR_Router::RouterID>);

            class RNETChan : public PWR_Router::ChanBase {
            public:

                RNETChan() {
                    DBGX("\n");
                }

                void add(EventChannel* chan) {
                    DBGX("\n");
                }

                void del(EventChannel* chan) {
                    DBGX("\n");
                }
            };
        private:
            void init(std::string, std::string);
            CommunicatorStore *m_commStore;
            Args m_args;
            ChannelSelect *m_chanSelect;
            EventChannel *m_routerChannel;
            EventId m_eventCounter;
        };

        class RNETRtrReturnEvent : public CommEvent {
        public:

            RNETRtrReturnEvent(SerialBuf & buf) {
                DBGX("\n");
            }
        };

        class RNETSelectData : public ChannelSelect::Data {
        public:

            RNETSelectData(EventChannel* chan) : m_chan(chan) {
            }
            virtual bool process(RNETClient*) = 0;

        protected:
            EventChannel* m_chan;
        };

        class RNETRouterData : public RNETSelectData {
        public:

            RNETRouterData(EventChannel* chan) : RNETSelectData(chan) {
            }

            bool process(RNETClient* gen) {
                Event* event = m_chan->getEvent();
                if (NULL == event) {
                    return true;
                } else {
                    if (event->process(gen, m_chan)) {
                        delete event;
                    }
                    return false;
                }
            }
        };
    }
}

#endif /* RNETCLIENT_H */

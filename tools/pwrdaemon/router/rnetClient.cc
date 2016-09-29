#include "rnetClient.h"

namespace RNET {

    namespace POWERAPI {

#ifdef USE_DEBUG
        static const char* _eventNames[] = {
            FOREACH_ENUM(GENERATE_STRING)
        };
#endif

        static void print_usage() {
            printf("%s()\n", __func__);
        }

        void initArgs(int argc, char *argv[], Args *args) {
            int opt = 0;
            int long_index = 0;

            enum {
                RTR_HOST, RTR_PORT
            };
            static struct option long_options[] = {
                {"routerHost", required_argument, NULL, RTR_HOST},
                {"routerPort", required_argument, NULL, RTR_PORT},
                {0, 0, 0, 0}
            };
            while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
                switch (opt) {
                    case RTR_HOST:
                        args->routerHost = optarg;
                        break;
                    case RTR_PORT:
                        args->routerPort = optarg;
                        break;
                    default:
                        print_usage();
                        exit(-1);
                }
            }
        }

        Event* allocRNETClientEvent(unsigned int type, SerialBuf& buf) {
            DBG4("PWR_Router", "`%s`\n", _eventNames[type]);

            switch (type) {
                default:
                    assert(0);
                case RNETLookupResp:
                    return new RNETLookupRespEvent(buf);
                case RNETRtrReturn:
                    return new RNETRtrReturnEvent(buf);
            }
        }

        RNETClient::RNETClient(int argc, char *argv[]) : m_chanSelect(NULL) {
            if (NULL != getenv("POWERAPI_DEBUG")) {
                _DbgFlags = atoi(getenv("POWERAPI_DEBUG"));
            }
            DBGX("\n");
            initArgs(argc, argv, &m_args);
            init(m_args.routerHost, m_args.routerPort);
        }

        RNETClient::RNETClient(std::string host, std::string port) : m_chanSelect(NULL) {
            init(host, port);
        }

        RNETClient::RNETClient(std::string host, unsigned int port) : m_chanSelect(NULL) {
            std::ostringstream oss;
            oss << port;
            init(host, oss.str());
        }

        RNETClient::~RNETClient() {
            DBGX("\n");
            delete m_commStore;
        }

        void RNETClient::init(std::string host, std::string port) {
            DBGX("\n");
            m_args.routerHost = host;
            m_args.routerPort = port;
            DBGX("routerHost=%s routerPort=%s\n", m_args.routerHost.c_str(), m_args.routerPort.c_str());
            this->m_chanSelect = getChannelSelect("TCP");
            std::string config = "server=" + m_args.routerHost + " serverPort=" + m_args.routerPort;
            DBGX("config:%s\n", config.c_str());
            this->m_routerChannel = getEventChannel("TCP", NULL, config, "router");
            m_commStore = new CommunicatorStore();
            this->m_eventCounter = 0;
        }

        void RNETClient::sendEvent(Event *ev) {
            DBGX("\n");
            ev->id = this->m_eventCounter++;
            this->m_routerChannel->sendEvent(ev);
        }

        void RNETClient::initCommunicator(std::string key, PWR_Router::RouterID rtrs[], unsigned int n) {
            RNETCommCreateEvent *evt = new RNETCommCreateEvent();
            COMM c;
            if (rtrs == NULL || n == 0) {
                DBGX("\n");
                c = this->m_commStore->newCOMM(0);
            } else {
                DBGX("\n");
                c = this->m_commStore->newCOMM(key);
                for (unsigned int i = 0; i < n; ++i) {
                    evt->children.push_back(rtrs[i]);
                }
            }
            c.label = COMM_START;
            this->m_commStore->put(c);
            evt->commID = c.ID;
            evt->commName = c.name;
            this->sendEvent(evt);
            delete evt;
            this->m_commStore->dump();
        }

        void RNETClient::initCommunicator(std::string key, std::vector<PWR_Router::RouterID> rtrIDs) {
            DBGX("\n");
            int n = rtrIDs.size();

            if (key == "WORLD" || n == 0) {
                this->initWORLD();
            } else {
                PWR_Router::RouterID rtrs[n];
                std::copy(rtrIDs.begin(), rtrIDs.end(), &rtrs[0]);
                this->initCommunicator(key, rtrs, n);
            }
        }

        void RNETClient::initWORLD() {
            this->initCommunicator("WORLD", NULL, 0);
        }

        void RNETClient::waitFor() {
            std::ostringstream configStream;
//            configStream << "server=";
//            configStream << getLocalHost();
//            configStream << " serverPort=";
            configStream << "listenPort=";
            configStream << 55000; // getAvailablePort();
            
            DBGX("%s\n", configStream.str().c_str());

            EventChannel *rtrChan = getEventChannel("TCP", allocRNETClientEvent, configStream.str(), "rnet-client");
            ChannelSelect *chanSelect = getChannelSelect("TCP");
            chanSelect->addChannel(rtrChan, new RNETRouterData(rtrChan));

            PWR_Router::SelectData* data;
            if ((data = static_cast<PWR_Router::SelectData*> (chanSelect->wait()))) {
                DBGX("Got a reply\n");
                delete data;
            }
        }

        void RNETClient::queryAndWait() {
            std::string host = getLocalHost();
            unsigned int port = getAvailablePort();
            DBGX("%s:%u\n", host.c_str(), port);
            RNETLookupEvent *ev = new RNETLookupEvent();
            ev->response.push_back(0);
            ev->host = host;
            ev->port = port;
            this->sendEvent(ev);
            delete ev;

            std::ostringstream oss;
            oss << "listenPort=";
            oss << port;

            RNETChan chan;
            EventChannel *channel = getEventChannel("TCP", allocRNETClientEvent, oss.str(), "client-listen");
            ChannelSelect *channelSelect = getChannelSelect("TCP");
            channelSelect->addChannel(channel, new PWR_Router::AcceptData<PWR_Router::EventData>(channel, &chan));

            PWR_Router::SelectData* data;

            DBGX("Waiting for replies\n");
            if ((data = static_cast<PWR_Router::SelectData*> (channelSelect->wait()))) {
                DBGX("Got a reply\n");
                //                if (data->process(channelSelect, NULL)) {
                //                    DBGX("reply processed\n");
                //                    delete data;
                //                }
            }

            delete channel;
            //delete channelSelect;
        }

        std::vector<PWR_Router::RouterID> RNETClient::lookupLeaves(std::vector<PWR_Router::RouterID> rtrIDs) {
            std::vector<PWR_Router::RouterID> response;
            this->queryAndWait();
            return response;
        }
    }
}

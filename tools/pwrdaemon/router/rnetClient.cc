#include "rnetClient.h"

namespace RNET {
    namespace POWERAPI {

        static void print_usage() {
            printf("%s()\n", __func__);
        }

        void initArgs(int argc, char* argv[], Args* args) {
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

        RNETClient::RNETClient(int argc, char* argv[]) : m_chanSelect(NULL) {
            if (NULL != getenv("POWERAPI_DEBUG")) {
                _DbgFlags = atoi(getenv("POWERAPI_DEBUG"));
            }
            DBGX("\n");
            initArgs(argc, argv, &m_args);
            DBGX("routerHost=%s routerPort=%s\n", m_args.routerHost.c_str(), m_args.routerPort.c_str());
            this->m_chanSelect = getChannelSelect("TCP");
            std::string config = "server=" + m_args.routerHost + " serverPort=" + m_args.routerPort;
            DBGX("config:%s\n", config.c_str());
            this->m_routerChannel = getEventChannel("TCP", NULL, config, "router");
            m_commStore = new CommunicatorStore();
        }

        RNETClient::RNETClient(std::string host, std::string port) : m_chanSelect(NULL) {
            DBGX("\n");
            m_args.routerHost = host;
            m_args.routerPort = port;
            DBGX("routerHost=%s routerPort=%s\n", m_args.routerHost.c_str(), m_args.routerPort.c_str());
            this->m_chanSelect = getChannelSelect("TCP");
            std::string config = "server=" + m_args.routerHost + " serverPort=" + m_args.routerPort;
            DBGX("config:%s\n", config.c_str());
            this->m_routerChannel = getEventChannel("TCP", NULL, config, "router");
            m_commStore = new CommunicatorStore();
        }

        RNETClient::~RNETClient() {
            DBGX("\n");
            delete m_commStore;
        }

        void RNETClient::sendEvent(Event* ev) {
            DBGX("\n");
            this->m_routerChannel->sendEvent(ev);
        }

        void RNETClient::initCommunicator(std::string key, PWR_Router::RouterID rtrs[], unsigned int n) {
            if (rtrs == NULL || n == 0) {
                DBGX("\n");
                RNETCommCreateEvent *evt = new RNETCommCreateEvent();
                COMM c = this->m_commStore->newCOMM(0);
                this->m_commStore->put(c);
                evt->commID = c.ID;
                evt->commName = c.name;
                this->sendEvent(evt);
                delete evt;
            } else {
                DBGX("\n");
                RNETCommCreateEvent *evt = new RNETCommCreateEvent();
                COMM c = this->m_commStore->newCOMM(key);
                this->m_commStore->put(c);
                evt->commID = c.ID;
                evt->commName = c.name;
                for (unsigned int i = 0; i < n; ++i) {
                    evt->children.push_back(rtrs[i]);
                }
                this->sendEvent(evt);
                delete evt;
            }
            this->m_commStore->dump();
        }

        void RNETClient::initWORLD() {
            this->initCommunicator("", NULL, 0);
        }
    }
}

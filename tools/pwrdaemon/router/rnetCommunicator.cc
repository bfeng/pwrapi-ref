#include "rnetCommunicator.h"

namespace RNET {

    namespace POWERAPI {

        CommunicatorStore::CommunicatorStore() {
            m_counter = 0;
        }

        CommunicatorStore::~CommunicatorStore() {
            m_store.clear();
        }

        COMM CommunicatorStore::newCOMM(CommID commID) {
            DBGX("\n");
            COMM c;
            m_counter = commID;
            c.ID = m_counter++;
            c.setGroup(c.ID);
            if (c.ID == 0) {
                c.name = "WORLD";
            } else {
                c.name = "UNNAME";
            }
            return c;
        }

        COMM CommunicatorStore::newCOMM(std::string name) {
            DBGX("\n");
            COMM c;
            c.ID = m_counter++;
            c.setGroup(c.ID);
            c.name = name;
            return c;
        }

        void CommunicatorStore::put(COMM c) {
            DBGX("\n");
            std::vector<COMM>::iterator it;
            for (it = m_store.begin(); it != m_store.end(); it++) {
                if ((*it).ID == c.ID) {
                    (*it).name = c.name;
                    return;
                }
            }
            m_store.push_back(c);
        }

        bool CommunicatorStore::has(COMM c) {
            DBGX("\n");
            std::vector<COMM>::iterator it;
            for (it = m_store.begin(); it != m_store.end(); it++) {
                if ((*it).ID == c.ID && (*it).name == c.name) {
                    return true;
                }
            }
            return false;
        }

        bool CommunicatorStore::remove(COMM c) {
            DBGX("\n");
            std::vector<COMM>::iterator it;
            for (it = m_store.begin(); it != m_store.end(); it++) {
                if ((*it).ID == c.ID && (*it).name == c.name) {
                    m_store.erase(it);
                    return true;
                }
            }
            return false;
        }

        void CommunicatorStore::dump() {
            DBGX("\n");
            std::vector<COMM>::iterator it;
            std::ostringstream buf;
            buf << "{";
            for (it = m_store.begin(); it != m_store.end(); ++it) {
                buf << "{";
                buf << "ID:" << (*it).ID;
                buf << ", Name:" << (*it).name;
                buf << ", Group:" << (*it).getGroup();
                buf << ", Label:" << LABEL_STR[(*it).label];
                buf << "}";
            }
            buf << "}";
            DBGX("%s\n", buf.str().c_str());
        }

        std::string getLocalHost() {
            char hostname[1024];
            hostname[1023] = '\0';
            int re = gethostname(hostname, 1023);
            if (re == 0) {
                return std::string(hostname);
            } else {
                return NULL;
            }
        }

        unsigned int getAvailablePort() {
            struct sockaddr_in serv_addr;
            unsigned int availablePort = 35000;
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                return -1;
            }
            while (1) {
                bzero((char *) &serv_addr, sizeof (serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = INADDR_ANY;
                serv_addr.sin_port = htons(availablePort);
                if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
                    availablePort++;
                    continue;
                }
                close(sockfd);
                break;
            }
            return availablePort;
        }
    }
}

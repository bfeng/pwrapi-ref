/*
 * File:   rnet_communicator.h
 * Author: bfeng
 *
 * Created on July 29, 2016, 1:10 PM
 */

#ifndef RNET_COMMUNICATOR_H
#define RNET_COMMUNICATOR_H

#include "debug.h"
#include "events.h"
#include <strings.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

namespace RNET {
    namespace POWERAPI {

        enum LABEL {
            COMM_START, COMM_INT, COMM_END
        };
        static const std::string LABEL_STR[] = {"COMM_START", "COMM_INT", "COMM_END"};

        class COMM {
        public:
            CommID ID;
            std::string name;
            LABEL label;

            void setGroup(unsigned int groupID) {
                this->groupID = groupID;
            }

            unsigned int getGroup() {
                return this->groupID;
            }
        private:
            unsigned int groupID;
        };

        class CommunicatorStore {
        public:
            CommunicatorStore();
            ~CommunicatorStore();
            COMM newCOMM(CommID commID);
            COMM newCOMM(std::string name);
            void put(COMM);
            bool has(COMM);
            bool remove(COMM);
            void dump();
        private:
            std::vector<COMM> m_store;
            CommID m_counter;
        };
        
        std::string getLocalHost();
        
        unsigned int getAvailablePort();
    }
}

#endif /* RNET_COMMUNICATOR_H */


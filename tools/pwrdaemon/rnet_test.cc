/*
 * File:   rnet_test.cc
 * Author: bfeng
 *
 * Created on July 15, 2016, 1:24 PM
 */

#include "rnetClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Args {
    int argc;
    std::vector<char *> argv;
};

void findArgs(std::string prefix, int argc, char **argv, Args &args) {
    prefix = "--" + prefix + ".";
    int len = prefix.size();

    // printf("prefix=%s\n",prefix.c_str());
    for (int i = 0; i < argc; i++) {
        if (0 == strncmp(argv[i], prefix.c_str(), len)) {

            // printf("'%s' -> ",argv[i]);
            memset(argv[i], '-', len);

            char *p = argv[i] + (len - 2);
            // printf("'%s'\n",p);
            args.argv.push_back(p);
        }
    }
    args.argc = args.argv.size();
    args.argv.push_back(NULL);
}

void createWorld(RNET::POWERAPI::RNETClient &client) {
    client.initWORLD();
}

void createSub1(RNET::POWERAPI::RNETClient &client) {
    PWR_Router::RouterID rtrs[1];
    rtrs[0] = 1;
    client.initCommunicator("test1", rtrs, 1);
}

void createSub2(RNET::POWERAPI::RNETClient &client) {
    std::vector<PWR_Router::RouterID> rtrIDs;
    rtrIDs.push_back(2);
    client.initCommunicator("test2", rtrIDs);
}

void testLookup(RNET::POWERAPI::RNETClient &client) {
    DBG4("RNET::POWERAPI", "START**********************************************************\n");
    std::vector<PWR_Router::RouterID> rtrIDs;
    rtrIDs.push_back(0);
    rtrIDs.push_back(1);
    rtrIDs.push_back(2);
    client.lookupLeaves(rtrIDs);
    DBG4("RNET::POWERAPI", "END************************************************************\n");
}

void testCommGet(RNET::POWERAPI::RNETClient &client) {
    DBG4("RNET::POWERAPI", "START**********************************************************\n");
    RNETCommGetReqEvent * ev = new RNETCommGetReqEvent();
    ev->args = "test-args-in-comm-get-power-data";
    client.sendEvent(ev);
    client.waitFor();
    delete ev;
    DBG4("RNET::POWERAPI", "END************************************************************\n");
}

int main(int argc, char *argv[]) {
    struct Args clientArgs;
    clientArgs.argv.push_back(argv[0]);
    findArgs("client", argc, argv, clientArgs);
    RNET::POWERAPI::RNETClient client(clientArgs.argc, &clientArgs.argv[0]);

//    createWorld(client);
//
//    createSub1(client);
//
//    createSub2(client);

//    testLookup(client);
    testCommGet(client);

    return (EXIT_SUCCESS);
}

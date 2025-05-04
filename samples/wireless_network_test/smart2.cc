/*
 * smart.cc
 *
 *  Created on: 19 feb. 2025
 *      Author: HP
 */

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box2: public cSimpleModule
{
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box2)

void box2::initialize()
{
    if (strcmp("box2a", getName()) == 0 ){
        cMessage *msg = new cMessage("Beacon");
        EV<<"Sending initial message \n";
        send(msg, "out");
    }
}

void box2::handleMessage(cMessage *msg)
{
    EV<<"Received message " << msg->getName() << " Sending again \n";
    send(msg, "out");
}

/*
 * smart.cc
 *
 *  Created on: 19 feb. 2025
 *      Author: HP
 */

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box4: public cSimpleModule
{
private:
    int counter;
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box4)

void box4::initialize()
{
    counter = par("limit");
    WATCH(counter);
    if (par("sendMsgOnInit").boolValue() == true){
        cMessage *msg = new cMessage("Beacon");
        EV<<"Sending initial message \n";
        send(msg, "out");
    }
}

void box4::handleMessage(cMessage *msg)
{
    counter --;
    if (counter == 0){
        EV<<"The counter "  << msg->getName() << " reach zero, deleting message. \n";
        delete msg;
    } else {
        EV<<"The counter is "  << counter << " sending back message. \n";
        send(msg, "out");
    }


}

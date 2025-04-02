/*
 * smart.cc
 *
 *  Created on: 19 feb. 2025
 *      Author: HP
 */

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box7: public cSimpleModule
{
private:
    int counter;
    cMessage *event;
    cMessage *beaconMsg;
public:
    box7();
    virtual ~box7();
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box7)

box7::box7()
{
    event = beaconMsg = nullptr;
}

box7::~box7()
{
    cancelAndDelete(event);
    delete beaconMsg;
}

void box7::initialize()
{
    event = new cMessage("event");
    beaconMsg = nullptr;

    if (strcmp("box7a", getName()) == 0){
        EV<<"Scheduling first send to 5.0s \n";
        beaconMsg = new cMessage ("beaconMsg");
        scheduleAt(5.0, event);
    }
}

void box7::handleMessage(cMessage *msg)
{
    if (msg == event){
        EV<<"Wait time is over, sending back the message \n";
        send(beaconMsg, "out");
        beaconMsg = NULL;
    } else {
        if (uniform(0,1) < 0.5){
            EV<<"Lost message \n";
            delete msg;
        } else {
            simtime_t delay = par("delayTime");
            EV<<"Message arrived, starting to wait 1s \n";
            beaconMsg = msg;
            scheduleAt(simTime() + delay, event);
        }
    }
}


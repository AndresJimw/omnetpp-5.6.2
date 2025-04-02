/*
 * smart.cc
 *
 *  Created on: 19 feb. 2025
 *      Author: HP
 */

#include <string.h>
#include <omnetpp.h>
#include <stdio.h>

using namespace omnetpp;

class box9a: public cSimpleModule
{
private:
    simtime_t timeout;
    cMessage *timeoutEvent;
    int seq;
    cMessage *message;
public:
    box9a();
    virtual ~box9a();
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
    virtual cMessage *generateNewMessage ();
    virtual void sendCopyOf(cMessage *msg);
};

Define_Module (box9a)

box9a::box9a()
{
    timeoutEvent = message = nullptr;
}

box9a::~box9a()
{
    cancelAndDelete(timeoutEvent);
    delete message;
}

void box9a::initialize()
{
    seq = 0;
    timeout = 1;
    timeoutEvent = new cMessage("timeoutEvent");
    EV<<"Sending Initial message \n";
    message = generateNewMessage();
    sendCopyOf(message);
    cMessage *msg = new cMessage("beaconMsg");
    send(msg, "out");
    scheduleAt(simTime()+timeout, timeoutEvent);
}

cMessage *box9a::generateNewMessage() {
    char msgname[20];
    sprintf(msgname, "Box9a-%d", ++seq);
    cMessage *msg = new cMessage(msgname);
    return msg;
}

void box9a::sendCopyOf(cMessage *msg) {
    cMessage *copy = (cMessage *)msg->dup();
    send(copy, "out");
}

void box9a::handleMessage(cMessage *msg)
{
    if (msg == timeoutEvent){
        EV<<"Timeout expired, re-sending the message \n";
        sendCopyOf(message);
        scheduleAt(simTime()+timeout, timeoutEvent);
    } else {
        EV<<"Message arrived \n";
        cancelEvent(timeoutEvent);
        delete message;
        message = generateNewMessage();
        sendCopyOf(message);
        scheduleAt(simTime()+timeout, timeoutEvent);
    }
}

class box9b: public cSimpleModule
{
protected:
    virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box9b)

void box9b::handleMessage(cMessage *msg)
{
    if(uniform(0,1) < 0.5){
        EV<<"Lost Message \n";
        bubble("Lost Message");
        delete msg;
    } else {
        EV<<"Sending back same message as ack";
        send(new cMessage("ack"), "out");
    }
}



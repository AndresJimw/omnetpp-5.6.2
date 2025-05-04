#include <string.h>
#include <omnetpp.h>
#include <stdio.h>
#include "box13_m.h"

using namespace omnetpp;

class box13: public cSimpleModule
{
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
    virtual void forwardMessage(boxMsg13 *msg);
    virtual boxMsg13 *generateMessage();
};

Define_Module (box13)

void box13::initialize()
{
    if (getIndex() == 0){
        boxMsg13 *msg = generateMessage();
        scheduleAt(0.0, msg);
    }
}

void box13::handleMessage(cMessage *msg)
{
    boxMsg13 *ttmsg = check_and_cast < boxMsg13 *> (msg);
    if (ttmsg -> getDestination() == getIndex()){
        EV<<"Message"<<msg<<" has arrived \n";
        bubble("Arrived starting new one\n");
        delete ttmsg;
        EV<<"Generating another message";
        boxMsg13 *newMessage = generateMessage();
        forwardMessage(newMessage);
    } else {
        forwardMessage(ttmsg);
    }
}

void box13::forwardMessage(boxMsg13 *msg)
{
    msg -> setHopCount(msg -> getHopCount()+1);
    int n = gateSize("out");
    int k = intuniform(0,n-1);
    EV<<"Forwarding message "<<msg<<" Port"<<k<<" .\n";
    send(msg, "gate$o", k);
}

boxMsg13 *box13::generateMessage()
{
    int src =getIndex();
    int n = getVectorSize();
    int dst = intuniform(0, n-2);

    if (dst >= src) {dst++;}

    char msgname[20];
    sprintf(msgname, "box-%d-to-%d", src, dst);
    boxMsg13 *msg = new boxMsg13(msgname);

    msg->setSource(src);
    msg->setDestination(dst);
    return msg;
}

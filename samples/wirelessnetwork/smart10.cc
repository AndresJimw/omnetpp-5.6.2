#include <string.h>
#include <omnetpp.h>
#include <stdio.h>

using namespace omnetpp;

class box10: public cSimpleModule
{
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
        virtual void forwardMessage (cMessage *msg);
};

Define_Module (box10)

void box10::initialize()
{
    if (getIndex() == 0){
        char msgname[20];
        sprintf(msgname, "box10-%d", getIndex());
        cMessage *msg = new cMessage (msgname);
        scheduleAt(0.0, msg);
    }
}


void box10::handleMessage(cMessage *msg){
    if (getIndex() == 3){
        EV<<"Message " <<msg<< " has arrived\n";
        delete msg;
    } else {
         forwardMessage(msg);
      }
}

void box10::forwardMessage(cMessage *msg)
{
    int n = gateSize("out");
    int k = intuniform(0, n-1);
    EV<<"Forwarding message "<<msg<<"Port"<<k<<".\n";
    send(msg, "out", k);

}

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box1: public cSimpleModule
{
protected:
    virtual void initialize () override;
    virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box1)

void box1::initialize()
{
    if (strcmp("box1a", getName())== 0 )
        {
            cMessage *msg = new cMessage ("Beacon");
            send(msg, "out");
        }
}

void box1::handleMessage(cMessage *msg)
{
    send(msg, "out");
}

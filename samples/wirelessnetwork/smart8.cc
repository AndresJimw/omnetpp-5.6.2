#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box8a: public cSimpleModule
{
    private:
        simtime_t timeout;
        cMessage *timeoutEvent;
    public:
        box8a();
        virtual ~box8a();
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box8a)

box8a::box8a()
{
    timeoutEvent = nullptr;
}

box8a::~box8a()
{
    cancelAndDelete(timeoutEvent);
}

void box8a::initialize()
{
    timeout = 1;
    timeoutEvent = new cMessage("timeoutEvent");
    EV <<"Sending initial message\n";
    cMessage *msg = new cMessage("beaconMsg");
    send(msg, "out");

    scheduleAt(simTime() + timeout, timeoutEvent);
}

void box8a::handleMessage(cMessage *msg){
    if (msg == timeoutEvent){
        EV<<"Time out expired, re-sending the message\n";
        cMessage *newMsg = new cMessage("beaconMsg");
        send(newMsg, "out");
        scheduleAt(simTime() + timeout, timeoutEvent);
    } else {
         EV<<"Message Arrive\n";
         cancelEvent(timeoutEvent);
         delete msg;
         cMessage *newMsg = new cMessage("beaconMsg");
         send(newMsg, "out");
         scheduleAt(simTime() + timeout, timeoutEvent);
      }
}

class box8b: public cSimpleModule
{
    protected:
        virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box8b)

void box8b::handleMessage(cMessage *msg){
    if (uniform(0,1) < 0.5){
        EV << "Lost Message";
        bubble("Lost Message");
        delete msg;
    } else {
        EV<<"Sending back same message as ack";
        send(msg, "out");
    }
}

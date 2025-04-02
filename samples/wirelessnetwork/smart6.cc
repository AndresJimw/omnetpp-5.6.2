#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box6: public cSimpleModule
{
    private:
        int counter;
        cMessage *event;
        cMessage *beaconMsg;
    public:
        box6();
        virtual ~box6();
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box6)

box6::box6()
{
    event = beaconMsg = nullptr;
}

box6::~box6()
{
    cancelAndDelete(event);
    delete beaconMsg;
}

void box6::initialize()
{

    event = new cMessage ("event");

    beaconMsg = nullptr;

    if (strcmp("box6a", getName()) == 0)
        {
            EV<<"Scheduling First send = 5.0s\n";
            beaconMsg = new cMessage ("beaconMsg");
            scheduleAt(5.0, event);
        }
}

void box6::handleMessage(cMessage *msg)
{

    if (msg == event){
        EV<<"The wait time is over, sending back the message\n";
        send(beaconMsg, "out");
        beaconMsg = NULL;
    } else {
        EV<<"Message arrived, starting to wait 1s\n";
        beaconMsg = msg;
        scheduleAt(simTime()+1, event);
    }

}

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box3: public cSimpleModule
{
    private:
        int counter;
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box3)

void box3::initialize()
{
    counter = 10;
    WATCH (counter);
    if (strcmp("box3a", getName())== 0 )
        {
            cMessage *msg = new cMessage ("Beacon");
            EV<<"Sending initial message\n";
            send(msg, "out");
        }
}

void box3::handleMessage(cMessage *msg)
{
    counter --;
    if (counter == 0){
        EV<<"The counter " << msg->getName() << " reach zero, deleting message.\n";
        delete msg;
    } else {
        EV<<"The counter is " << counter<< ". Sending back message\n";
        send(msg, "out");
    }

}

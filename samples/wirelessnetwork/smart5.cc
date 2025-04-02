#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class box5: public cSimpleModule
{
    private:
        int counter;
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
};

Define_Module (box5)

void box5::initialize()
{
    counter = par("limit");
    WATCH (counter);
    if (par("sendMsgOnInit").boolValue() == true)
        {
            cMessage *msg = new cMessage ("Beacon");
            EV<<"Sending initial message\n";
            send(msg, "out");
        }
}

void box5::handleMessage(cMessage *msg)
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

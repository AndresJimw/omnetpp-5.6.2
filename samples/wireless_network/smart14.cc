#include <string.h>
#include <omnetpp.h>
#include <stdio.h>
#include "box14_m.h"

using namespace omnetpp;

class box14: public cSimpleModule
{
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
        virtual void forwardMessage (boxMsg14 *msg);
        virtual boxMsg14 *generateMessage();
        virtual void refreshDisplay() const override;
    private:
        long numSent;
        long numReceived;
};

Define_Module (box14)

void box14::initialize()
{
    numSent = 0;
    numReceived = 0;
    WATCH(numSent);
    WATCH(numReceived);
    if (getIndex() == 0){
        boxMsg14 *msg = generateMessage();
        numSent++;
        scheduleAt(0.0, msg);
    }

}


void box14::handleMessage(cMessage *msg){
    boxMsg14 *ttmsg = check_and_cast < boxMsg14 *> (msg);

    if (ttmsg -> getDestination() == getIndex()){
        numReceived++;
        EV<<"Message " <<msg<< " has arrived\n";
        bubble("Arrived, starting new one\n");
        delete ttmsg;
        EV<<"Generating another message";
        boxMsg14 *newMessage = generateMessage();
        forwardMessage(newMessage);
        numSent++;


    } else {
         forwardMessage(ttmsg);
      }
}

void box14::forwardMessage(boxMsg14 *msg)
{
    msg -> setHopCount(msg -> getHopCount()+1);
    int n = gateSize("gate");
    int k = intuniform(0, n-1);
    EV<<"Forwarding message "<<msg<<"Port"<<k<<".\n";
    send(msg, "gate$o", k);

}

boxMsg14 *box14::generateMessage(){
    int src = getIndex();
    int n = getVectorSize();
    int dst = intuniform(0, n-2);

    if (dst >= src){dst++;}

    char msgname[20];
    sprintf(msgname, "box-%d-to-%d", src, dst);
    boxMsg14 *msg = new boxMsg14(msgname);

    msg -> setSource(src);
    msg -> setDestination(dst);

    return msg;
}

void box14::refreshDisplay() const {
    char buf[40];
    sprintf(buf,"rcvd: %ld sent: %ld", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

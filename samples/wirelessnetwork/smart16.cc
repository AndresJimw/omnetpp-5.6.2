#include <string.h>
#include <omnetpp.h>
#include <stdio.h>
#include "box16_m.h"

using namespace omnetpp;

class box16: public cSimpleModule
{
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
        virtual void forwardMessage (boxMsg16 *msg);
        virtual boxMsg16 *generateMessage();
    private:
        simsignal_t arrivalSignal;
};

Define_Module (box16)

void box16::initialize()
{
    arrivalSignal = registerSignal("arrival");

    if (getIndex() == 0){
        boxMsg16 *msg = generateMessage();

        scheduleAt(0.0, msg);
    }

}


void box16::handleMessage(cMessage *msg){
    boxMsg16 *ttmsg = check_and_cast < boxMsg16 *> (msg);

    if (ttmsg -> getDestination() == getIndex()){
        int hopCount = ttmsg->getHopCount();
        emit(arrivalSignal, hopCount);
        EV<<"Message " <<msg<< " has arrived after " <<hopCount<< " hops\n";

        bubble("Arrived, starting new one\n");
        delete ttmsg;
        EV<<"Generating another message";
        boxMsg16 *newMessage = generateMessage();
        forwardMessage(newMessage);

    } else {
         forwardMessage(ttmsg);
      }
}

void box16::forwardMessage(boxMsg16 *msg)
{
    msg -> setHopCount(msg -> getHopCount()+1);
    int n = gateSize("gate");
    int k = intuniform(0, n-1);
    EV<<"Forwarding message "<<msg<<"Port"<<k<<".\n";
    send(msg, "gate$o", k);

}

boxMsg16 *box16::generateMessage(){
    int src = getIndex();
    int n = getVectorSize();
    int dst = intuniform(0, n-2);

    if (dst >= src){dst++;}

    char msgname[20];
    sprintf(msgname, "box-%d-to-%d", src, dst);
    boxMsg16 *msg = new boxMsg16(msgname);

    msg -> setSource(src);
    msg -> setDestination(dst);

    return msg;
}














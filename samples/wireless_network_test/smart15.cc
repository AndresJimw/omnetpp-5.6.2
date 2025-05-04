#include <string.h>
#include <omnetpp.h>
#include <stdio.h>
#include "box15_m.h"

using namespace omnetpp;

class box15: public cSimpleModule
{
    protected:
        virtual void initialize () override;
        virtual void handleMessage (cMessage *msg) override;
        virtual void forwardMessage (boxMsg15 *msg);
        virtual boxMsg15 *generateMessage();
        virtual void finish() override;
    private:
        long numSent;
        long numReceived;
        cHistogram hopCountStats;
        cOutVector hopCountVector;
};

Define_Module (box15)

void box15::initialize()
{
    numReceived = 0;
    numSent = 0;
    WATCH(numSent);
    WATCH(numReceived);
    hopCountStats.setName("hopCountStat");
    hopCountVector.setName("hopCountVector");
    if (getIndex() == 0){
        boxMsg15 *msg = generateMessage();

        scheduleAt(0.0, msg);
    }

}


void box15::handleMessage(cMessage *msg){
    boxMsg15 *ttmsg = check_and_cast < boxMsg15 *> (msg);

    if (ttmsg -> getDestination() == getIndex()){
        int hopCount = ttmsg->getHopCount();
        EV<<"Message " <<msg<< " has arrived after " <<hopCount<< " hops\n";
        numReceived++;
        hopCountVector.record(hopCount);
        hopCountStats.collect(hopCount);
        bubble("Arrived, starting new one\n");
        delete ttmsg;
        EV<<"Generating another message";
        boxMsg15 *newMessage = generateMessage();
        forwardMessage(newMessage);
        numSent++;
    } else {
         forwardMessage(ttmsg);
      }
}

void box15::forwardMessage(boxMsg15 *msg)
{
    msg -> setHopCount(msg -> getHopCount()+1);
    int n = gateSize("gate");
    int k = intuniform(0, n-1);
    EV<<"Forwarding message "<<msg<<"Port"<<k<<".\n";
    send(msg, "gate$o", k);

}

boxMsg15 *box15::generateMessage(){
    int src = getIndex();
    int n = getVectorSize();
    int dst = intuniform(0, n-2);

    if (dst >= src){dst++;}

    char msgname[20];
    sprintf(msgname, "box-%d-to-%d", src, dst);
    boxMsg15 *msg = new boxMsg15(msgname);

    msg -> setSource(src);
    msg -> setDestination(dst);

    return msg;
}

void box15::finish(){
    EV<<"sent: "<< numSent <<endl;
    EV<<"received: " << numReceived <<endl;
    EV<<"hopCount, min: " << hopCountStats.getMin() << endl;
    EV<<"hopCount, max: " << hopCountStats.getMax() << endl;
    EV<<"hopCount, mean: " << hopCountStats.getMean() << endl;
    EV<<"hopCount, stddev: " << hopCountStats.getStddev() << endl;
    recordScalar("#sent", numSent);
    recordScalar("#sent", numReceived);
    hopCountStats.recordAs("hopCount");

}

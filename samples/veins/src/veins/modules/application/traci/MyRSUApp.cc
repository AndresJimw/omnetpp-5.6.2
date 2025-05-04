//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <veins/modules/application/traci/MyRSUApp.h>
#include <veins/modules/application/traci/TraCIDemo11pMessage_m.h>

using namespace veins;

Define_Module(veins::MyRSUApp);

void MyRSUApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        // Initializing members and pointers of your application goes here
        EV << "Initializing " << par("appName").stringValue() << std::endl;
        sentMessage = false;
    }
    else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }
}

void MyRSUApp::finish()
{
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void MyRSUApp::onBSM(DemoSafetyMessage* bsm)
{
    // Your application has received a beacon message from another car or RSU
    // code for handling the message goes here
    EV<<"Llego un beacon con Psid" << bsm->getPsid() << ".\n";
    EV<<"Beacon con velocidad: " << sqrt(bsm->getSenderSpeed().squareLength()) << "m/s .\n";
    EV<<"Beacon de un vehiculo localizado a: " << bsm->getSenderPos().distance(curPosition);

    int d = int (bsm->getPsid());
    double velocidad = sqrt(bsm->getSenderSpeed().squareLength());
    double distancia = bsm->getSenderPos().distance(curPosition);

    if (listBeacon.SearchBeacon(d)){
        listBeacon.UpdateBeacon(d, bsm->getArrivalTime(), 0, d, velocidad, 0, 0, 0, 0, distancia, 0, 0, 0);
    } else {
        listBeacon.AddBeacon(d, bsm->getArrivalTime(), 0, d, velocidad, 0, 0, 0, 0, distancia, 0, 0, 0);
    }
    EV << "TABLA DE VECINOS";
    double time_live = 5;
    count = listBeacon.PurgeBeacons(time_live);
    listBeacon.PrintBeacons();
}

void MyRSUApp::onWSM(BaseFrame1609_4* frame)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    findHost()->getDisplayString().setTagArg("1", 1, "green");

    if (!sentMessage) {
        sentMessage = true;
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime()+uniform(0.01,0.2), wsm->dup());
    }
}

void MyRSUApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void MyRSUApp::handleSelfMsg(cMessage* msg)
{
    DemoBaseApplLayer::handleSelfMsg(msg);
    // this method is for self messages (mostly timers)
    // it is important to call the DemoBaseApplLayer function for BSM and WSM transmission
}

void MyRSUApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);
    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}

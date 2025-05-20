#include "veins/modules/application/traci/trad/TrADApplLayer.h"
#include <vector>

#define CLUSTER_ANGLE_THRESHOLD 10  // en grados

using namespace veins;

void TrADApplLayer::initialize(int stage)
{
    BaseApplLayer::initialize(stage);

    if (stage == 0) {

        // initialize pointers to other modules
        if (FindModule<TraCIMobility*>::findSubModule(getParentModule())) {
            mobility = TraCIMobilityAccess().get(getParentModule());
            traci = mobility->getCommandInterface();
            traciVehicle = mobility->getVehicleCommandInterface();
        }
        else {
            traci = nullptr;
            mobility = nullptr;
            traciVehicle = nullptr;
        }

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        mac = FindModule<DemoBaseApplLayerToMac1609_4Interface*>::findSubModule(getParentModule());
        ASSERT(mac);

        // read parameters
        headerLength = par("headerLength");
        sendBeacons = par("sendBeacons").boolValue();
        beaconLengthBits = par("beaconLengthBits");
        beaconUserPriority = par("beaconUserPriority");
        beaconInterval = par("beaconInterval");

        dataLengthBits = par("dataLengthBits");
        dataOnSch = par("dataOnSch").boolValue();
        dataUserPriority = par("dataUserPriority");

        wsaInterval = par("wsaInterval").doubleValue();
        currentOfferedServiceId = -1;

        isParked = false;

        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        findHost()->subscribe(TraCIMobility::parkingStateChangedSignal, this);

        sendBeaconEvt = new cMessage("beacon evt", SEND_BEACON_EVT);
        sendWSAEvt = new cMessage("wsa evt", SEND_WSA_EVT);

        generatedBSMs = 0;
        generatedWSAs = 0;
        generatedWSMs = 0;
        receivedBSMs = 0;
        receivedWSAs = 0;
        receivedWSMs = 0;
    }
    else if (stage == 1) {

        // store MAC address for quick access
        myId = mac->getMACAddress();

        // simulate asynchronous channel access

        if (dataOnSch == true && !mac->isChannelSwitchingActive()) {
            dataOnSch = false;
            EV_ERROR << "App wants to send data on SCH but MAC doesn't use any SCH. Sending all data on CCH" << std::endl;
        }
        simtime_t firstBeacon = simTime();

        if (par("avoidBeaconSynchronization").boolValue() == true) {

            simtime_t randomOffset = dblrand() * beaconInterval;
            firstBeacon = simTime() + randomOffset;

            if (mac->isChannelSwitchingActive() == true) {
                if (beaconInterval.raw() % (mac->getSwitchingInterval().raw() * 2)) {
                    EV_ERROR << "The beacon interval (" << beaconInterval << ") is smaller than or not a multiple of  one synchronization interval (" << 2 * mac->getSwitchingInterval() << "). This means that beacons are generated during SCH intervals" << std::endl;
                }
                firstBeacon = computeAsynchronousSendingTime(beaconInterval, ChannelType::control);
            }

            if (sendBeacons) {
                scheduleAt(firstBeacon, sendBeaconEvt);
            }
        }
    }
}

simtime_t TrADApplLayer::computeAsynchronousSendingTime(simtime_t interval, ChannelType chan)
{

    /*
     * avoid that periodic messages for one channel type are scheduled in the other channel interval
     * when alternate access is enabled in the MAC
     */

    simtime_t randomOffset = dblrand() * interval;
    simtime_t firstEvent;
    simtime_t switchingInterval = mac->getSwitchingInterval(); // usually 0.050s
    simtime_t nextCCH;

    /*
     * start event earliest in next CCH (or SCH) interval. For alignment, first find the next CCH interval
     * To find out next CCH, go back to start of current interval and add two or one intervals
     * depending on type of current interval
     */

    if (mac->isCurrentChannelCCH()) {
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw()) + switchingInterval * 2;
    }
    else {
        nextCCH = simTime() - SimTime().setRaw(simTime().raw() % switchingInterval.raw()) + switchingInterval;
    }

    firstEvent = nextCCH + randomOffset;

    // check if firstEvent lies within the correct interval and, if not, move to previous interval

    if (firstEvent.raw() % (2 * switchingInterval.raw()) > switchingInterval.raw()) {
        // firstEvent is within a sch interval
        if (chan == ChannelType::control) firstEvent -= switchingInterval;
    }
    else {
        // firstEvent is within a cch interval, so adjust for SCH messages
        if (chan == ChannelType::service) firstEvent += switchingInterval;
    }

    return firstEvent;
}

void TrADApplLayer::populateWSM(BaseFrame1609_4* wsm, LAddress::L2Type rcvId, int serial)
{
    wsm->setRecipientAddress(rcvId);
    wsm->setBitLength(headerLength);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        // Posición y velocidad actual
        bsm->setSenderPos(curPosition);
        bsm->setSenderSpeed(curSpeed);

        // ID único del beacon (puede ser simplemente un contador local por ahora)
        static int beaconCounter = 0;
        bsm->setBeaconId(beaconCounter++);

        // Dirección en grados (heading)
        double heading = atan2(curSpeed.y, curSpeed.x) * 180.0 / M_PI;
        if (heading < 0) heading += 360;
        bsm->setHeading(heading);

        // Número de vecinos (provisional: se implementará más adelante con recepción de beacons)
        bsm->setNeighborCount(0);

        // Channel Busy Ratio (provisional: igual a 0.0, se calcula más adelante)
        bsm->setCbr(0.0);

        // Lista de mensajes ya recibidos (provisional: vacía por ahora)
        bsm->setMessageListArraySize(0);

        // Otros parámetros estándar
        bsm->setPsid(-1);
        bsm->setChannelNumber(static_cast<int>(Channel::cch));
        bsm->addBitLength(beaconLengthBits);
        wsm->setUserPriority(beaconUserPriority);
    }
    else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        wsa->setChannelNumber(static_cast<int>(Channel::cch));
        wsa->setTargetChannel(static_cast<int>(currentServiceChannel));
        wsa->setPsid(currentOfferedServiceId);
        wsa->setServiceDescription(currentServiceDescription.c_str());
    }
    else {
        if (dataOnSch)
            wsm->setChannelNumber(static_cast<int>(Channel::sch1)); // will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        else
            wsm->setChannelNumber(static_cast<int>(Channel::cch));
        wsm->addBitLength(dataLengthBits);
        wsm->setUserPriority(dataUserPriority);
    }
}

void TrADApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent();
    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
    else if (signalID == TraCIMobility::parkingStateChangedSignal) {
        handleParkingUpdate(obj);
    }
}

void TrADApplLayer::handlePositionUpdate(cObject* obj)
{
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getPositionAt(simTime());
    curSpeed = mobility->getCurrentSpeed();
}

void TrADApplLayer::handleParkingUpdate(cObject* obj)
{
    isParked = mobility->getParkingState();
}

void TrADApplLayer::handleLowerMsg(cMessage* msg)
{

    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    ASSERT(wsm);

    if (DemoSafetyMessage* bsm = dynamic_cast<DemoSafetyMessage*>(wsm)) {
        receivedBSMs++;
        onBSM(bsm);
    }
    else if (DemoServiceAdvertisment* wsa = dynamic_cast<DemoServiceAdvertisment*>(wsm)) {
        receivedWSAs++;
        onWSA(wsa);
    }
    else {
        receivedWSMs++;
        onWSM(wsm);
    }

    delete (msg);
}

void TrADApplLayer::handleSelfMsg(cMessage* msg)
{
    switch (msg->getKind()) {
    case SEND_BEACON_EVT: {
        purgeOldNeighbors();  // limpiar vecinos antes de generar beacon nuevo

        DemoSafetyMessage* bsm = new DemoSafetyMessage();
        populateWSM(bsm);

        // Prueba de clasificación direccional
        auto clusters = classifyDirectionalClusters();
        EV_INFO << "Numero de clusters direccionales: " << clusters.size() << endl;

        for (size_t i = 0; i < clusters.size(); ++i) {
            EV_INFO << "  Sector " << i << " contiene " << clusters[i].size() << " nodos: ";
            for (LAddress::L2Type id : clusters[i]) {
                EV_INFO << id << " ";
            }
            EV_INFO << endl;
        }

        sendDown(bsm);
        scheduleAt(simTime() + beaconInterval, sendBeaconEvt);
        break;
    }
    case SEND_WSA_EVT: {
        DemoServiceAdvertisment* wsa = new DemoServiceAdvertisment();
        populateWSM(wsa);
        sendDown(wsa);
        scheduleAt(simTime() + wsaInterval, sendWSAEvt);
        break;
    }
    default: {
        if (msg) EV_WARN << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
        break;
    }
    }
}

void TrADApplLayer::finish()
{
    recordScalar("generatedWSMs", generatedWSMs);
    recordScalar("receivedWSMs", receivedWSMs);

    recordScalar("generatedBSMs", generatedBSMs);
    recordScalar("receivedBSMs", receivedBSMs);

    recordScalar("generatedWSAs", generatedWSAs);
    recordScalar("receivedWSAs", receivedWSAs);
}

TrADApplLayer::~TrADApplLayer()
{
    cancelAndDelete(sendBeaconEvt);
    cancelAndDelete(sendWSAEvt);
    findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
}

void TrADApplLayer::startService(Channel channel, int serviceId, std::string serviceDescription)
{
    if (sendWSAEvt->isScheduled()) {
        throw cRuntimeError("Starting service although another service was already started");
    }

    mac->changeServiceChannel(channel);
    currentOfferedServiceId = serviceId;
    currentServiceChannel = channel;
    currentServiceDescription = serviceDescription;

    simtime_t wsaTime = computeAsynchronousSendingTime(wsaInterval, ChannelType::control);
    scheduleAt(wsaTime, sendWSAEvt);
}

void TrADApplLayer::stopService()
{
    cancelEvent(sendWSAEvt);
    currentOfferedServiceId = -1;
}

void TrADApplLayer::sendDown(cMessage* msg)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDown(msg);
}

void TrADApplLayer::sendDelayedDown(cMessage* msg, simtime_t delay)
{
    checkAndTrackPacket(msg);
    BaseApplLayer::sendDelayedDown(msg, delay);
}

void TrADApplLayer::checkAndTrackPacket(cMessage* msg)
{
    if (dynamic_cast<DemoSafetyMessage*>(msg)) {
        EV_TRACE << "sending down a BSM" << std::endl;
        generatedBSMs++;
    }
    else if (dynamic_cast<DemoServiceAdvertisment*>(msg)) {
        EV_TRACE << "sending down a WSA" << std::endl;
        generatedWSAs++;
    }
    else if (dynamic_cast<BaseFrame1609_4*>(msg)) {
        EV_TRACE << "sending down a wsm" << std::endl;
        generatedWSMs++;
    }
}

void TrADApplLayer::onBSM(DemoSafetyMessage* bsm) {
    LAddress::L2Type senderId = bsm->getSenderModuleId();

    // Guardar o reemplazar beacon del vecino
    if (neighborTable.find(senderId) != neighborTable.end()) {
        delete neighborTable[senderId];  // borrar beacon viejo
    }
    neighborTable[senderId] = bsm->dup();  // guardar copia nueva con tiempo actual

    EV_INFO << "[TrAD] Nodo " << myId
            << " recibio beacon de " << senderId
            << " en t=" << simTime()
            << " | Total vecinos: " << neighborTable.size() << endl;

}

void TrADApplLayer::purgeOldNeighbors() {
    simtime_t now = simTime();
    std::vector<LAddress::L2Type> toDelete;

    for (const auto& entry : neighborTable) {
        DemoSafetyMessage* bsm = entry.second;
        if (now - bsm->getTimestamp() > neighborTimeout) {
            toDelete.push_back(entry.first);
        }
    }

    for (LAddress::L2Type id : toDelete) {
        delete neighborTable[id];
        neighborTable.erase(id);
    }
}

std::vector<std::vector<LAddress::L2Type>> TrADApplLayer::classifyDirectionalClusters() {
    std::vector<std::vector<LAddress::L2Type>> clusters;

    // Copia de la tabla de vecinos para clasificación
    std::map<LAddress::L2Type, DemoSafetyMessage*> remainingNeighbors = neighborTable;

    while (!remainingNeighbors.empty()) {
        // Elegimos el primer vecino para iniciar un nuevo cluster
        auto it = remainingNeighbors.begin();
        LAddress::L2Type refId = it->first;
        Coord refPos = it->second->getSenderPos();
        Coord vecRef = refPos - curPosition;

        std::vector<LAddress::L2Type> currentCluster;
        currentCluster.push_back(refId);
        remainingNeighbors.erase(it);

        // Iteramos sobre los demás vecinos
        for (auto it2 = remainingNeighbors.begin(); it2 != remainingNeighbors.end(); ) {
            Coord pos = it2->second->getSenderPos();
            Coord vec = pos - curPosition;

            // Calculamos ángulo entre ref y actual
            double angle = acos((vecRef * vec) / (vecRef.length() * vec.length())) * (180.0 / M_PI);

            if (angle < CLUSTER_ANGLE_THRESHOLD) {
                currentCluster.push_back(it2->first);
                it2 = remainingNeighbors.erase(it2);
            } else {
                ++it2;
            }
        }

        clusters.push_back(currentCluster);
    }

    // Debug
    EV_INFO << "[TrAD] Nodo " << myId << " clasifico " << clusters.size() << " clusters direccionales" << endl;
    int sector = 0;
    for (auto& cluster : clusters) {
        EV_INFO << "  Sector " << sector++ << " contiene " << cluster.size() << " nodos: ";
        for (auto id : cluster) EV_INFO << id << " ";
        EV_INFO << endl;
    }

    return clusters;
}



Define_Module(TrADApplLayer);

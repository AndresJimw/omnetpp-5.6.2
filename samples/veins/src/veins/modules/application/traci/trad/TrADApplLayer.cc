#include "veins/modules/application/traci/trad/TrADApplLayer.h"
#include <vector>
#include <set>

#define CLUSTER_ANGLE_THRESHOLD 10  // en grados
#define SEND_REBROADCAST_EVT 999
#define BITRATE 6e6  // 6 Mbps (como en el paper de TrAD)

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

        bitrate = par("bitrate").doubleValue();

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
        // Posicion y velocidad actual
        bsm->setSenderPos(curPosition);
        bsm->setSenderSpeed(curSpeed);

        // ID unico del beacon (contador local)
        static int beaconCounter = 0;
        bsm->setBeaconId(beaconCounter++);

        // Direccion en grados (heading)
        double heading = atan2(curSpeed.y, curSpeed.x) * 180.0 / M_PI;
        if (heading < 0) heading += 360;
        bsm->setHeading(heading);

        // Numero de vecinos (opcional por ahora)
        bsm->setNeighborCount(neighborTable.size());

        // Channel Busy Ratio (igual a 0.0, se calcula mas adelante)
        bsm->setCbr(channelBusyRatio);

        // Clasificación y lista de prioridad
        auto clusters = classifyDirectionalClusters();
        auto priorityList = buildPriorityListFromClusters(clusters);

        // Guardar lista de prioridad
        bsm->setPriorityListArraySize(priorityList.size());
        for (size_t i = 0; i < priorityList.size(); ++i) {
            bsm->setPriorityList(i, priorityList[i]);
        }

        // Selección de SCF-agents por cluster (coordinator y breaker)
        std::vector<LAddress::L2Type> scfAgents;
        for (const auto& cluster : clusters) {
            auto pair = selectSCFAgents(cluster);
            if (pair.first != -1) scfAgents.push_back(pair.first);   // coordinator
            if (pair.second != -1) scfAgents.push_back(pair.second); // breaker
        }

        // Guardar lista de SCF-agents en el beacon
        bsm->setScfAgentsArraySize(scfAgents.size());
        for (size_t i = 0; i < scfAgents.size(); ++i) {
            bsm->setScfAgents(i, scfAgents[i]);
        }

        // Debug para SCF-agents
        EV_INFO << "[TrAD] Nodo " << myId << " incluyo " << scfAgents.size()
                << " SCF-agents en beaconId " << bsm->getBeaconId() << "\n";

        // Lista de mensajes ya conocidos por este nodo
        bsm->setMessageListArraySize(receivedMessageIds.size());
        int idx = 0;
        for (int msgId : receivedMessageIds) {
            bsm->setMessageList(idx++, msgId);
        }

        // Otros parametros estandar
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

    // Acumula tiempo ocupado para calcular CBR
    if (simTime() - cbrWindowStart >= cbrWindowLength) {
        channelBusyRatio = busyTime / cbrWindowLength.dbl();
        busyTime = 0.0;
        cbrWindowStart = simTime();
    }

    if (BITRATE > 0) {
        simtime_t duration = wsm->getBitLength() / BITRATE;
        busyTime += duration.dbl();
    }

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

        // Clasificacion direccional
        auto clusters = classifyDirectionalClusters();
        EV_INFO << "[TrAD] Nodo " << myId << " clasifico " << clusters.size() << " clusters direccionales\n";

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
    case SEND_REBROADCAST_EVT: {
        DemoSafetyMessage* rebroadcast = check_and_cast<DemoSafetyMessage*>(msg);
        EV_INFO << "[TrAD] Nodo " << myId << " retransmite beaconId " << rebroadcast->getBeaconId() << "\n";
        sendDown(rebroadcast);
        break;
    }
    default: {
        if (msg) EV_WARN << "APP: Mensaje interno desconocido: " << msg->getName() << endl;
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

    // Guarda o reemplaza beacon del vecino
    if (neighborTable.find(senderId) != neighborTable.end()) {
        delete neighborTable[senderId];  // borra beacon viejo
    }
    neighborTable[senderId] = bsm->dup();  // guarda copia nueva con tiempo actual

    EV_INFO << "[TrAD] Nodo " << myId
            << " recibio beacon de " << senderId
            << " en t=" << simTime()
            << " | Total vecinos: " << neighborTable.size() << endl;

    // Muestra lista de prioridad recibida
    EV_INFO << "[TrAD] Prioridad recibida:\n";
    for (size_t i = 0; i < bsm->getPriorityListArraySize(); ++i) {
        EV_INFO << "  [" << i << "] Nodo " << bsm->getPriorityList(i) << "\n";
    }

    // --- Retransmision adaptativa basada en prioridad (TrAD Alg. 2) ---

    int beaconId = bsm->getBeaconId();
    lastReceivedBeaconId = beaconId;

    // Si ya recibimos este mensaje antes, ignorar
    if (receivedMessageIds.count(beaconId)) {
        EV_INFO << "[TrAD] Nodo " << myId << " ya recibió beaconId " << beaconId << ". No retransmite.\n";
        return;
    }

    // Verifica si algun mensaje en la lista ya fue visto
    for (int i = 0; i < bsm->getMessageListArraySize(); ++i) {
        if (bsm->getMessageList(i) == beaconId) {
            EV_INFO << "[TrAD] Nodo " << myId << " detecto beaconId " << beaconId << " en messageList[]. No retransmite.\n";
            return;
        }
    }

    receivedMessageIds.insert(beaconId);  // registra este beacon como recibido

    // Verifica si este nodo esta en la lista de prioridad
    int position = -1;
    for (size_t i = 0; i < bsm->getPriorityListArraySize(); ++i) {
        if (bsm->getPriorityList(i) == myId) {
            position = i;
            break;
        }
    }

    // Si no está en la lista, no retransmite
    if (position == -1) {
        EV_INFO << "[TrAD] Nodo " << myId << " no esta en la lista de prioridad. No retransmite.\n";
        return;
    }

    // Verifica si este nodo esta en la lista de SCF-agents
    bool isScfAgent = false;
    for (int i = 0; i < bsm->getScfAgentsArraySize(); ++i) {
        if (bsm->getScfAgents(i) == myId) {
            isScfAgent = true;
            break;
        }
    }

    if (!isScfAgent) {
        EV_INFO << "[TrAD] Nodo " << myId << " no esta en la lista SCF-agents. No retransmite.\n";
        return;
    }

    // Si pasa todas las condiciones, programa la retransmision

    // Calcula delay segun posicion (mayor posicion => mayor delay)
    double maxDelay = 0.1; // 100 ms como en el paper (TrAD Algorithm 2)
    simtime_t delay = SimTime(position * maxDelay / bsm->getPriorityListArraySize());

    EV_INFO << "[TrAD] Nodo " << myId << " programara retransmision del beaconId " << beaconId
            << " con delay = " << delay << "s (posicion " << position << " de "
            << bsm->getPriorityListArraySize() << ")\n";

    // Clonar el beacon y programar la retransmision
    DemoSafetyMessage* rebroadcast = bsm->dup();
    rebroadcast->setKind(SEND_REBROADCAST_EVT);
    scheduleAt(simTime() + delay, rebroadcast);
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

    // Copia de la tabla de vecinos para clasificacion
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

        // Iteramos sobre los demas vecinos
        for (auto it2 = remainingNeighbors.begin(); it2 != remainingNeighbors.end(); ) {
            Coord pos = it2->second->getSenderPos();
            Coord vec = pos - curPosition;

            // Calculamos angulo entre ref y actual
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

// Metodo: calcular UTX para un cluster
std::map<LAddress::L2Type, double> TrADApplLayer::calculateUTX(const std::vector<LAddress::L2Type>& cluster) {
    std::map<LAddress::L2Type, double> utxMap;

    // Numero de vecinos (normalizado con maximo = 25)
    int numNeighbors = neighborTable.size();
    double N = std::min(static_cast<double>(numNeighbors) / 25.0, 1.0);

    for (const auto& neighborId : cluster) {
        DemoSafetyMessage* bsm = neighborTable[neighborId];
        if (!bsm) continue;

        // Distancia al vecino (normalizada con maxRadioRange = 366 m)
        double distance = curPosition.distance(bsm->getSenderPos());
        double D = std::min(distance / 366.0, 1.0);

        // Channel Busy Ratio reportado por el vecino
        double CBR = bsm->getCbr();

        // Peso segun la congestion del canal (wCBR)
        double wCBR;
        if (CBR < 0.6)
            wCBR = 1.0;
        else if (CBR < 0.8)
            wCBR = 1.0 - CBR;
        else
            wCBR = 0.001;

        // Utilidad UTX
        double utx = wCBR * (N + D) / 2.0;
        utxMap[neighborId] = utx;
    }

    return utxMap;
}

// Metodo: ordenar por utilidad
std::vector<LAddress::L2Type> TrADApplLayer::sortByUTX(const std::map<LAddress::L2Type, double>& utxMap) {
    std::vector<std::pair<LAddress::L2Type, double>> vec(utxMap.begin(), utxMap.end());

    std::sort(vec.begin(), vec.end(),
              [](const std::pair<LAddress::L2Type, double>& a, const std::pair<LAddress::L2Type, double>& b) {
                  return a.second > b.second;  // orden descendente por utilidad
              });

    std::vector<LAddress::L2Type> sorted;
    for (const auto& p : vec) {
        sorted.push_back(p.first);
    }

    return sorted;
}

// Metodo: buildPriorityListFromClusters
std::vector<LAddress::L2Type> TrADApplLayer::buildPriorityListFromClusters(const std::vector<std::vector<LAddress::L2Type>>& clusters) {
    std::vector<std::vector<LAddress::L2Type>> orderedClusters;

    // Ordenar cada cluster individualmente por UTX
    for (const auto& cluster : clusters) {
        auto utxMap = calculateUTX(cluster);
        auto sorted = sortByUTX(utxMap);
        orderedClusters.push_back(sorted);
    }

    std::vector<LAddress::L2Type> priorityList;
    size_t round = 0;
    bool added;

    do {
        added = false;
        for (auto& cluster : orderedClusters) {
            if (round < cluster.size()) {
                priorityList.push_back(cluster[round]);
                added = true;
            }
        }
        round++;
    } while (added);

    return priorityList;
}

std::pair<LAddress::L2Type, LAddress::L2Type> TrADApplLayer::selectSCFAgents(const std::vector<LAddress::L2Type>& cluster) {
    if (cluster.empty()) return {-1, -1};

    // Primero calcula la utilidad UTX de cada nodo
    auto utxMap = calculateUTX(cluster);

    // Luego selecciona como coordinador al nodo con mayor UTX
    LAddress::L2Type coordinator = sortByUTX(utxMap).front();

    // Por ultimo selecciona como breaker al nodo mas lejano del coordinador
    Coord coordPos = neighborTable[coordinator]->getSenderPos();
    LAddress::L2Type breaker = coordinator;
    double maxDistance = -1;

    for (const auto& nodeId : cluster) {
        if (nodeId == coordinator) continue;
        Coord pos = neighborTable[nodeId]->getSenderPos();
        double dist = coordPos.distance(pos);
        if (dist > maxDistance) {
            maxDistance = dist;
            breaker = nodeId;
        }
    }

    EV_INFO << "[TrAD][SCF] Cluster con " << cluster.size() << " nodos -> "
            << "Coordinator: " << coordinator << ", Breaker: " << breaker << "\n";

    return {coordinator, breaker};
}

std::map<LAddress::L2Type, double> TrADApplLayer::calculateUSCF(const std::vector<LAddress::L2Type>& scfCandidates) {
    std::map<LAddress::L2Type, double> uscfMap;

    // Normalizacion: maximo 25 vecinos
    double N = std::min(static_cast<double>(neighborTable.size()) / 25.0, 1.0);

    for (const auto& nid : scfCandidates) {
        DemoSafetyMessage* bsm = neighborTable[nid];
        if (!bsm) continue;

        // Distancia al nodo candidato
        double distance = curPosition.distance(bsm->getSenderPos());
        double D = std::min(distance / 366.0, 1.0); // 366 m es el radio maximo segun paper

        // Angulo relativo al emisor original
        Coord vecSelf = curSpeed;
        Coord vecToCandidate = bsm->getSenderPos() - curPosition;

        double angle = acos((vecSelf * vecToCandidate) / (vecSelf.length() * vecToCandidate.length())) * (180.0 / M_PI);
        double A = std::min(angle / 180.0, 1.0); // Normalizado a [0,1]

        // Congestion reportada por el candidato
        double CBR = bsm->getCbr();

        double wCBR;
        if (CBR < 0.6)
            wCBR = 1.0;
        else if (CBR < 0.8)
            wCBR = 1.0 - CBR;
        else
            wCBR = 0.001;

        double uscf = wCBR * (N + D + A) / 3.0;
        uscfMap[nid] = uscf;
    }

    return uscfMap;
}

Define_Module(TrADApplLayer);

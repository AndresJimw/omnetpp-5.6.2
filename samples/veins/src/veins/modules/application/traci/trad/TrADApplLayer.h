#pragma once

#ifndef VEINS_TRADAPPLAYER_H_
#define VEINS_TRADAPPLAYER_H_

#include <map>

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/BaseFrame1609_4_m.h"
#include "veins/modules/messages/DemoServiceAdvertisement_m.h"
#include "veins/modules/messages/DemoSafetyMessage_m.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/DemoBaseApplLayerToMac1609_4Interface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {

using veins::AnnotationManager;
using veins::AnnotationManagerAccess;
using veins::TraCICommandInterface;
using veins::TraCIMobility;
using veins::TraCIMobilityAccess;

/**
 * @brief
 * Demo application layer base class.
 *
 * @author Brandon Jim�nez
 *
 * @ingroup applLayer
 *
 * @see DemoBaseApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class VEINS_API TrADApplLayer : public BaseApplLayer {

public:
    ~TrADApplLayer() override;
    void initialize(int stage) override;
    void finish() override;

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

    enum MessageKinds {
        SEND_BEACON_EVT,
        SEND_WSA_EVT
    };

protected:
    /** @brief handle messages from below and calls the onWSM, onBSM, and onWSA functions accordingly */
    void handleLowerMsg(cMessage* msg) override;

    /** @brief handle self messages */
    void handleSelfMsg(cMessage* msg) override;

    /** @brief Clasifica los vecinos actuales en clusters direccionales usando �ngulo vectorial */
    std::vector<std::vector<LAddress::L2Type>> classifyDirectionalClusters();

    /** @brief sets all the necessary fields in the WSM, BSM, or WSA. */
    virtual void populateWSM(BaseFrame1609_4* wsm, LAddress::L2Type rcvId = LAddress::L2BROADCAST(), int serial = 0);

    /** @brief this function is called upon receiving a BaseFrame1609_4 */
    virtual void onWSM(BaseFrame1609_4* wsm){};

    /** @brief this function is called upon receiving a DemoSafetyMessage, also referred to as a beacon  */
    virtual void onBSM(DemoSafetyMessage* bsm);

    /** @brief Elimina beacons de vecinos que han expirado */
    void purgeOldNeighbors();

    /** @brief this function is called upon receiving a DemoServiceAdvertisement */
    virtual void onWSA(DemoServiceAdvertisment* wsa){};

    /** @brief this function is called every time the vehicle receives a position update signal */
    virtual void handlePositionUpdate(cObject* obj);

    /** @brief this function is called every time the vehicle parks or starts moving again */
    virtual void handleParkingUpdate(cObject* obj);

    /** @brief This will start the periodic advertising of the new service on the CCH
     *
     *  @param channel the channel on which the service is provided
     *  @param serviceId a service ID to be used with the service
     *  @param serviceDescription a literal description of the service
     */
    virtual void startService(Channel channel, int serviceId, std::string serviceDescription);

    /** @brief stopping the service and advertising for it */
    virtual void stopService();

    /** @brief compute a point in time that is guaranteed to be in the correct channel interval plus a random offset
     *
     * @param interval the interval length of the periodic message
     * @param chantype the type of channel, either type_CCH or type_SCH
     */
    virtual simtime_t computeAsynchronousSendingTime(simtime_t interval, ChannelType chantype);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent. Must be a WSM/BSM/WSA
     */
    virtual void sendDown(cMessage* msg);

    /**
     * @brief overloaded for error handling and stats recording purposes
     *
     * @param msg the message to be sent. Must be a WSM/BSM/WSA
     * @param delay the delay for the message
     */
    virtual void sendDelayedDown(cMessage* msg, simtime_t delay);

    /**
     * @brief helper function for error handling and stats recording purposes
     *
     * @param msg the message to be checked and tracked
     */
    virtual void checkAndTrackPacket(cMessage* msg);

protected:
    /* pointers ill be set when used with TraCIMobility */
    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;

    AnnotationManager* annotations;
    DemoBaseApplLayerToMac1609_4Interface* mac;

    /* support for parking currently only works with TraCI */
    bool isParked;

    /* BSM (beacon) settings */
    uint32_t beaconLengthBits;
    uint32_t beaconUserPriority;
    simtime_t beaconInterval;
    bool sendBeacons;

    /* WSM (data) settings */
    uint32_t dataLengthBits;
    uint32_t dataUserPriority;
    bool dataOnSch;

    /* WSA settings */
    int currentOfferedServiceId;
    std::string currentServiceDescription;
    Channel currentServiceChannel;
    simtime_t wsaInterval;

    /* state of the vehicle */
    Coord curPosition;
    Coord curSpeed;
    LAddress::L2Type myId = 0;
    int mySCH;

    /* stats */
    uint32_t generatedWSMs;
    uint32_t generatedWSAs;
    uint32_t generatedBSMs;
    uint32_t receivedWSMs;
    uint32_t receivedWSAs;
    uint32_t receivedBSMs;

    /** @brief Variables para calcular Channel Busy Ratio (CBR) */
    simtime_t cbrWindowStart = 0;
    simtime_t cbrWindowLength = 0.1; // 100 ms
    double busyTime = 0.0;
    double channelBusyRatio = 0.0;

    /** @brief Bitrate utilizado para calcular el tiempo de ocupación del canal (CBR) */
    double bitrate = 6e6;  // valor por defecto, se sobreescribe por .ned o .ini

    /* Header length in bits for all outgoing messages (used in populateWSM) */
    uint32_t headerLength;

    std::map<LAddress::L2Type, DemoSafetyMessage*> neighborTable;
    simtime_t neighborTimeout = 1.5; // tiempo m�ximo antes de eliminar un vecino (paper: 1.5s)

    /** @brief Set local para evitar retransmitir mensajes ya recibidos (por beaconId) */
    std::set<int> receivedMessageIds;

    /** @brief Mapa que registra qué nodos ya retransmitieron cada beaconId */
    std::map<int, std::set<LAddress::L2Type>> rebroadcastsByBeaconId;

    /** @brief ID de beacon más reciente recibido (por debug o control opcional) */
    int lastReceivedBeaconId = -1;

    /** @brief Construye lista de prioridad usando round-robin y utilidad UTX */
    std::vector<LAddress::L2Type> buildPriorityListFromClusters(const std::vector<std::vector<LAddress::L2Type>>& clusters);

    /** @brief Calcula la utilidad UTX de cada nodo en un cluster */
    std::map<LAddress::L2Type, double> calculateUTX(const std::vector<LAddress::L2Type>& cluster);

    /** @brief Ordena nodos segun su utilidad UTX de forma descendente */
    std::vector<LAddress::L2Type> sortByUTX(const std::map<LAddress::L2Type, double>& utxMap);

    /** @brief Calcula la utilidad USCF para una lista de posibles SCF-agents */
    std::map<LAddress::L2Type, double> calculateUSCF(const std::vector<LAddress::L2Type>& scfCandidates);

    /** @brief Selecciona el coordinador y breaker SCF en un cluster */
    std::pair<LAddress::L2Type, LAddress::L2Type> selectSCFAgents(const std::vector<LAddress::L2Type>& cluster);

    /* messages for periodic events such as beacon and WSA transmissions */
    cMessage* sendBeaconEvt;
    cMessage* sendWSAEvt;
};

} // namespace veins

#endif

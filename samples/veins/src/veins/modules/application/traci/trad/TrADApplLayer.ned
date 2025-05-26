package org.car2x.veins.modules.application.traci.trad;
import org.car2x.veins.base.modules.IBaseApplLayer;


// Application layer for the TrAD protocol implementation.

// This module is the entry point of the TrAD dissemination logic.
// It handles beaconing, message suppression, SCF coordination, and
// vector-based directional classification as defined in the original paper.


simple TrADApplLayer like IBaseApplLayer
{
    parameters:
        @class(veins::TrADApplLayer);
        int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(true); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconUserPriority = default(7); //the user priority (UP) of the beacon messages
        double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataUserPriority = default(7); //the default user priority (UP) for data packets

        bool avoidBeaconSynchronization = default(true); //don't start beaconing directly after node was created but delay to avoid artifical synchronization

        bool sendWSA = default(false); // unused in TrAD, leave false unless needed
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval =  default(1s) @unit(s);
        double bitrate @unit(bps) = default(6Mbps); // PHY bitrate usado para calcular CBR
        double gpsDriftSigma = default(2.5m); // Desviación estándar del error GPS
        bool enableGpsDrift = default(true);  // Activar/desactivar error GPS
        bool isSourceNode = default(false);
        double initialDataDelay @unit(s) = default(1s);  // Tiempo de inicio para nodos fuente

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}

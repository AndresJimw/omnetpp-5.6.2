import veins.base.utils.Coord;
import veins.modules.messages.BaseFrame1609_4;
import veins.base.utils.SimpleAddress;

namespace veins;

packet DemoSafetyMessage extends BaseFrame1609_4 {
    Coord senderPos;         // Global GPS Position
    Coord senderSpeed;
    int beaconId;
    double heading;          // Direccion de movimiento en grados
    int neighborCount;       // Numero de vecinos inmediatos
    double cbr;              // Channel Busy Ratio [0, 1]
    int priorityList[];      // Lista de nodos ordenados por prioridad UTX
    int messageList[];       // Lista de IDs de mensajes almacenados por el nodo
    int scfAgents[];         // SCF-agents: coordinators y breakers
    int serial;  // identificador local incremental
}

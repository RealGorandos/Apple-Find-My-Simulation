#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include <unordered_map>
#include "UdpBasicAppAux.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/common/geometry/common/Coord.h"
namespace inet {

class INET_API ServerApp : public UdpBasicAppAux {
    protected:
        typedef UdpBasicAppAux super;

        struct airtagData {
            L3Address ip;
            int isDeviceFound;
            simtime_t ts;
            Coord deviceLocation;
        };
        std::unordered_map<std::string, airtagData> airtagDatabase;
        std::unordered_map<std::string, airtagData> iphoneDatabase;
    protected:
        void initialize(int stage) override;
        void socketDataArrived(UdpSocket *socket, Packet *packet) override;
        void finish() override;

    protected:
        int successfulLookupAirTagCount = 0;
        int unsuccessfulLookupAirTagCount = 0;
        simsignal_t successfulLookupAirTag;
        simsignal_t unsuccessfulLookupAirTag;

        int successfulLookupIphoneCount = 0;
        int unsuccessfulLookupIphoneCount = 0;
        simsignal_t successfulLookupIphone;
        simsignal_t unsuccessfulLookupIphone;

        simsignal_t endtoenddelay;
        simsignal_t senderHostId;
        simsignal_t receiverHostId;
};


} /* namespace inet */

#endif /* SERVERAPP_H_ */

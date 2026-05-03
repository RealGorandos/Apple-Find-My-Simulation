#ifndef __INET_CLIENTAPP_H
#define __INET_CLIENTAPP_H

#include <unordered_map>
#include "UdpBasicAppAux.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

class INET_API ClientApp : public UdpBasicAppAux
{
    protected:
        typedef UdpBasicAppAux super;

        struct addressEntry {
            L3Address ip;
            simtime_t ts;
        };

        struct airtagData {
            simtime_t ts;
            float distance;
        };
        std::unordered_map<std::string, airtagData> unkownAirtagList;
        std::unordered_map<std::string, addressEntry> addressBook;
        std::vector<std::string> friendAirtagList;
        std::vector<std::string> friendIphoneList;
    protected:
        void friendListsInit();

        void sendLookupRequest();
        void sendLookupRequest(std::string deviceName, int packet_type);

        void sendPhoneLocation();

        std::string randomSelectForSendingLocation(std::string deviceName);
        void sendLocationToIphone(std::string deviceName, Coord deviceLocation);
        
        void sendPacket() override;
        void initialize(int stage) override;
        void socketDataArrived(UdpSocket *socket, Packet *packet) override;

        void finish() override;
    protected:
        simsignal_t chunkLength;
        simsignal_t endtoenddelay;
        simsignal_t senderHostId;
        simsignal_t receiverHostId;
        int unkownAirtagFollowCount = 0;
        simsignal_t unkownAirtagFollow;

        int friendlyMessagesCount = 0;
        simsignal_t friendlyMessages;
};

} // namespace inet

#endif

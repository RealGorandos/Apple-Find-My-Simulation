#ifndef __INET_SENDLOCATIONAPP_H
#define __INET_SENDLOCATIONAPP_H

#include "UdpBasicAppAux.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
namespace inet {

class INET_API SendLocationApp : public UdpBasicAppAux
{
    protected:
        typedef UdpBasicAppAux super;
        L3Address destAddress;
        Coord CurrPos;
    protected:
        void findClosesAppleDevice();
        void initialize(int stage) override;
        void sendPacket() override;
        void finish() override;
};

} // namespace inet

#endif

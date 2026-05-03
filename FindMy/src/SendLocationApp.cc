#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "DataPacket_m.h"
#include <vector>

#include "SendLocationApp.h"

namespace inet {

Define_Module(SendLocationApp);

void SendLocationApp::initialize(int stage)
{
    super::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
//        findClosesAppleDevice();
    }
}

void SendLocationApp::findClosesAppleDevice(){
    cTopology topo("topo");

    topo.extractByNedTypeName(cStringTokenizer("FindMy.AppleDeviceNode").asVector());

    std::vector< IMobility *> posVect;

    cModule *airTagNode = getContainingNode(this);
    IMobility  *airTagMod = check_and_cast<IMobility *>(airTagNode->getSubmodule("mobility"));
    CurrPos = airTagMod->getCurrentPosition();
    double minDist = 5000;
    for (int i = 0; i < topo.getNumNodes(); i++)
    {

        cTopology::Node *destNode = topo.getNode(i);

        IMobility *destNodeMod;

        cModule *host = destNode->getModule();
        destNodeMod = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        Coord destNodeCoord = destNodeMod->getCurrentPosition();
        double dist = CurrPos.distance(destNodeCoord);
        if(dist < minDist){
            minDist = dist;
            destAddress = inet::L3AddressResolver().addressOf(host);
        }
        EV_INFO << "CURRENT NODE ITERATION IS: " << host << ", ITs POSITION IS: " << destNodeCoord << ", ITs ADDRESS IS: " << destAddress << ", "<< ownName << " POSITION IS: " << CurrPos << ". THE DISTANCE IS: " << dist << endl ;

    }




}

void SendLocationApp::sendPacket()
{
    int k = intrand(100);

    if (k < 50) {

        findClosesAppleDevice();

        EV_INFO << "AIRTAG LOCATION REQUEST CREATED BY: " << ownName << "Trying to send Packet to destAddress: " << destAddress ;
        Packet *packet = new Packet("AirTag Location");
        packet->addTag<FragmentationReq>()->setDontFragment(true);

        const auto& payload = makeShared<DataPacket>();
        payload->setChunkLength(B(20));
        payload->setType(0);
        payload->setDeviceName(ownName.c_str());
        payload->setIsDeviceFound(1);

        payload->setDeviceLocation(CurrPos);

        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddress, destPort);
        numSent++;
    }
}

void SendLocationApp::finish() {
    super::finish();
}

} // namespace inet

#include "ServerApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TimeTag_m.h"
#include "DataPacket_m.h"

namespace inet {

Define_Module(ServerApp);

void ServerApp::initialize(int stage)
{
    super::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        successfulLookupAirTagCount = 0;
        unsuccessfulLookupAirTagCount = 0;
        successfulLookupAirTag = registerSignal("successfulLookupAirTag");
        unsuccessfulLookupAirTag = registerSignal("unsuccessfulLookupAirTag");

        successfulLookupIphoneCount = 0;
        unsuccessfulLookupIphoneCount = 0;
        successfulLookupIphone = registerSignal("successfulLookupIphone");
        unsuccessfulLookupIphone = registerSignal("unsuccessfulLookupIphone");

        endtoenddelay = registerSignal("endtoenddelay");

        senderHostId = registerSignal("senderHostId");
        receiverHostId = registerSignal("receiverHostId");
    }
}

void ServerApp::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    int senderHostIdInt = 0;
    int receivedHostIdInt = 0;

    emit(senderHostId, senderHostIdInt);
    emit(receiverHostId, receivedHostIdInt);

    emit(endtoenddelay, simTime() - pk->getCreationTime());

    emit(packetReceivedSignal, pk);
    numReceived++;

    L3Address remoteAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    int srcPort = pk->getTag<L4PortInd>()->getSrcPort();
    pk->clearTags();
    pk->trim();

    auto currentTime = simTime();

    auto data = pk->peekData<DataPacket>();
    auto packetType = data->getType();
    auto deviceName = data->getDeviceName();
    if (packetType == 0) { // Airtag Location Packet
        EV_INFO << "SERVER RECIEVED AN AIRTAG LOCATION" << endl;

        airtagDatabase[deviceName].isDeviceFound = data->getIsDeviceFound();
        airtagDatabase[deviceName].deviceLocation = data->getDeviceLocation();
        airtagDatabase[deviceName].ip = remoteAddress;
        airtagDatabase[deviceName].ts = simTime();
        EV_INFO << "--- SERVER: HEARTBEAT RECEIVED FROM: " << deviceName << endl;
    } else if (packetType == 3) { // iPhone Location Packet
        EV_INFO << "SERVER RECIEVED AN IPHONE LOCATION" << endl;

        iphoneDatabase[deviceName].isDeviceFound = data->getIsDeviceFound();
        iphoneDatabase[deviceName].deviceLocation = data->getDeviceLocation();
        iphoneDatabase[deviceName].ip = remoteAddress;
        iphoneDatabase[deviceName].ts = simTime();
        EV_INFO << "--- SERVER: HEARTBEAT RECEIVED FROM: " << deviceName << endl;
    } else if (packetType == 1) { // Airtag Lookup Request
        EV_INFO << "SERVER PACKET IS Airtag Lookup Request" << endl;

        Packet *packet = new Packet("Airtag Lookup Response");
        packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto& payload = makeShared<DataPacket>();
        payload->setChunkLength(B(300));
        payload->setType(2);  // Airtag Lookup Response
        

        if (airtagDatabase.find(deviceName) == airtagDatabase.end()) { // Not Exists
            payload->setDeviceName(deviceName);
            payload->setIsDeviceFound(-1);
            EV_INFO << "--- SERVER: AIRTAG NOT FOUND: " << deviceName << endl;
            unsuccessfulLookupAirTagCount++;
            emit(unsuccessfulLookupAirTag, unsuccessfulLookupAirTagCount);
        }
        else { // Exists
            payload->setDeviceName(deviceName);
            payload->setIsDeviceFound(airtagDatabase[deviceName].isDeviceFound);
            payload->setDeviceLocation(airtagDatabase[deviceName].deviceLocation);
            payload->setIp(airtagDatabase[deviceName].ip);
            EV_INFO << "--- SERVER: AIRTAG FOUND: " << deviceName << endl;
            successfulLookupAirTagCount++;
            emit(successfulLookupAirTag, successfulLookupAirTagCount);
        }

        packet->insertAtBack(payload);
        socket->sendTo(packet, remoteAddress, srcPort);

        emit(packetSentSignal, packet);
        numSent++;
    }
    else if (packetType == 4) { // iPhone Lookup Request
        EV_INFO << "SERVER PACKET IS iPhone Lookup Request" << endl;

        Packet *packet = new Packet("iPhone lookup Response");
        packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto& payload = makeShared<DataPacket>();
        payload->setChunkLength(B(300));
        payload->setType(5);  // iPhone Lookup Response
        

        if (iphoneDatabase.find(deviceName) == iphoneDatabase.end()) { // Not Exists
            payload->setDeviceName(deviceName);
            payload->setIsDeviceFound(-1);
            EV_INFO << "--- SERVER: iPhone NOT FOUND: " << deviceName << endl;
            unsuccessfulLookupIphoneCount++;
            emit(unsuccessfulLookupIphone, unsuccessfulLookupIphoneCount);
        }
        else { // Exists
            payload->setDeviceName(deviceName);
            payload->setIsDeviceFound(iphoneDatabase[deviceName].isDeviceFound);
            payload->setDeviceLocation(iphoneDatabase[deviceName].deviceLocation);
            payload->setIp(iphoneDatabase[deviceName].ip);
            EV_INFO << "--- SERVER: iPhone FOUND: " << deviceName << endl;
            successfulLookupIphoneCount++;
            emit(successfulLookupIphone, successfulLookupIphoneCount);
        }

        packet->insertAtBack(payload);
        socket->sendTo(packet, remoteAddress, srcPort);

        emit(packetSentSignal, packet);
        numSent++;
    }
    delete pk;
}

void ServerApp::finish() {
    recordScalar("successfulLookupAirTag", successfulLookupAirTag);
    recordScalar("unsuccessfulLookupAirTag", unsuccessfulLookupAirTag);

    recordScalar("successfulLookupIphone", successfulLookupIphone);
    recordScalar("unsuccessfulLookupIphone", unsuccessfulLookupIphone);
    super::finish();
}

} /* namespace inet */

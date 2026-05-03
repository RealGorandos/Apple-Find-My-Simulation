#include "ClientApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "DataPacket_m.h"

#include <algorithm>
#include <random>
#include <regex>
#include <string>

namespace inet {

Define_Module(ClientApp);

void ClientApp::friendListsInit() {
    const char* airtag_ch1 = par("airtag_prefix");
    std::string airtag_prefix(airtag_ch1);

    const char* iphone_ch1 = par("apple_device_prefix");
    std::string apple_device_prefix(iphone_ch1);

    int airtag_number = par("airtag_number");
    int apple_host_number = par("apple_host_number");
    int size_iphones = par("friendly_iphones_num");
    int size_airtags = par("friendly_airtags_num");

    std::vector<std::string> airtags_hosts;
    std::vector<std::string> iphones_hosts;

    for (int i = 0; i < airtag_number; i++) {
        std::stringstream tmp;
        tmp << airtag_prefix << "[" << i << "]";
        std::string result;
        tmp >> result;
        airtags_hosts.push_back(result);
    }

    for (int i = 0; i < apple_host_number; i++) {
        std::stringstream tmp;
        tmp << apple_device_prefix << "[" << i << "]";
        std::string result;
        tmp >> result;

        if (strcmp(result.c_str(), ownName.c_str()) != 0) {
            iphones_hosts.push_back(result);
        }
    }
    
    EV_INFO << "-------" << ownName << "'s DEVICES FRIEND List-------" << endl;
    std::shuffle(std::begin(iphones_hosts), std::end(iphones_hosts), std::default_random_engine(intrand(10000)));

    EV_INFO << iphones_hosts.size() << endl;
    EV_INFO << size_iphones << endl;
    for (int i = 0; i < size_iphones; i++) {
        friendIphoneList.push_back(iphones_hosts[i]);
        EV_INFO << iphones_hosts[i] << endl;
    }

    EV_INFO << "-------" << ownName << "'s AIRTAG FRIEND List-------" << endl;

    std::shuffle(std::begin(airtags_hosts), std::end(airtags_hosts), std::default_random_engine(intrand(10000)));

    EV_INFO << airtags_hosts.size() << endl;
    EV_INFO << size_airtags << endl;
    for (int i = 0; i < size_airtags; i++) {
        friendAirtagList.push_back(airtags_hosts[i]);
        EV_INFO << airtags_hosts[i] << endl;
    }

}

void ClientApp::initialize(int stage)
{
    super::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        chunkLength = registerSignal("chunkLength");
        friendListsInit();
        unkownAirtagFollowCount = 0;
        unkownAirtagFollow = registerSignal("unkownAirtagFollow");

        friendlyMessagesCount = 0;
        friendlyMessages = registerSignal("friendlyMessages");
        endtoenddelay = registerSignal("endtoenddelay");

        senderHostId = registerSignal("senderHostId");
        receiverHostId = registerSignal("receiverHostId");
    }
}

void ClientApp::sendLookupRequest() {
    int probability = intrand(100);
    std::string  deviceName;
    int packet_type;
    if (probability < 50){
        int k = intrand(friendAirtagList.size());
        deviceName = friendAirtagList[k];
        packet_type = 1;
    }else{
        int k = intrand(friendIphoneList.size());
        deviceName = friendIphoneList[k];
        packet_type = 4;
    }
    if (deviceName.size() > 0) {
        sendLookupRequest(deviceName, packet_type);
    }
}

void ClientApp::sendLookupRequest(std::string deviceName, int packet_type) {
    EV_INFO << "LOOKUP REQUEST: " << ownName << "---->" << deviceName << endl;
    Packet *packet = new Packet("Lookup Request");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<DataPacket>();
    payload->setChunkLength(B(20));
    payload->setType(packet_type);

    payload->setDeviceName(deviceName.c_str());

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddress, destPort);
    numSent++;
}

void ClientApp::sendPhoneLocation() {
    cModule *iphoneNode = getContainingNode(this);
    IMobility  *iphoneMod = check_and_cast<IMobility *>(iphoneNode->getSubmodule("mobility"));
    Coord CurrPos = iphoneMod->getCurrentPosition();

    Packet *packet = new Packet("iPhone Location");
    packet->addTag<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<DataPacket>();
    payload->setChunkLength(B(20));
    payload->setType(3);
    payload->setDeviceName(ownName.c_str());
    payload->setIsDeviceFound(1);

    payload->setDeviceLocation(CurrPos);

    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddress, destPort);
    EV_INFO << "IPHONE: " << ownName << ". SENT LOCATION: " << CurrPos << ". To THE SERVER"<< endl;
    numSent++;
}
void ClientApp::sendPacket()
{
    int k = intrand(100);

    if (k < 25) {
        sendLookupRequest();
    }else if (k >= 25 && k < 50){
        sendPhoneLocation();
    }
}


std::string ClientApp::randomSelectForSendingLocation(std::string deviceName) {
    std::vector<std::string> addresses;
    for (const auto& pair : addressBook) {
        if (strcmp(pair.first.c_str(), deviceName.c_str()) != 0) {
            addresses.push_back(pair.first);
        }
    }
    if (addresses.empty()) {
        return "";
    }
    std::shuffle(std::begin(addresses), std::end(addresses), std::default_random_engine(intrand(10000)));

    return addresses[0];
}

void ClientApp::sendLocationToIphone(std::string deviceName, Coord deviceLocation){

    std::string receiverDeviceName = randomSelectForSendingLocation(deviceName);
    if (receiverDeviceName.size() > 0){
        std::string specific_airtag = "airtag[3]";
        EV_INFO << "IPHONE: " << ownName << ", SHARING LOCATION OF: " << deviceName << ", TO IPHONE: " << receiverDeviceName;
        Packet *packet = new Packet("Friendly Location Share");
        packet->addTag<FragmentationReq>()->setDontFragment(true);

        const auto& payload = makeShared<DataPacket>();
        int packetSize = 20 + intrand(980);
        payload->setChunkLength(B(packetSize));
        emit(chunkLength, packetSize);
        payload->setType(6);
        payload->setDeviceName(deviceName.c_str());
        std::string msg = "IPHONE: " + receiverDeviceName + ", RECEIVED LOCATION OF: " + deviceName + ", FROM IPHONE: " + ownName;
        payload->setTextMessage(msg.c_str());
        payload->setDeviceLocation(deviceLocation);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        EV_INFO << ownName << "'s FRIENDS LIST----------" << endl;
        for (const auto& pair : friendIphoneList) {
            EV_INFO << pair << endl;
        }
        EV_INFO << ownName << "'s ADDRESS LIST----------" << endl;
        for (const auto& pair : addressBook) {
            EV_INFO << pair.first << endl;
        }
        emit(packetSentSignal, packet);
        socket.sendTo(packet, addressBook[receiverDeviceName].ip, 5555);
        numSent++;
    }
}

void ClientApp::socketDataArrived(UdpSocket *socket, Packet *pk)
{


    emit(endtoenddelay, simTime() - pk->getCreationTime());

    emit(packetReceivedSignal, pk);
    numReceived++;

    auto data = pk->peekData<DataPacket>();

    auto packetType = data->getType();
    std::string deviceName = std::string(data->getDeviceName());
    auto isDeviceFound = data->getIsDeviceFound();
    auto deviceLocation = data->getDeviceLocation();
    auto ip = data->getIp();

    int senderHostIdInt = -1;
    int receivedHostIdInt = -1;

    cModule *currPhoneLocation = getContainingNode(this);
    IMobility  *phoneMod = check_and_cast<IMobility *>(currPhoneLocation->getSubmodule("mobility"));
    Coord CurrPos = phoneMod->getCurrentPosition();

    if (packetType == 0){
        std::string specific_airtag = "airtag[3]";
        bool isFriend = std::find(friendAirtagList.begin(), friendAirtagList.end(), deviceName) != friendAirtagList.end();
        bool existInForiegnList = unkownAirtagList.find(deviceName) != unkownAirtagList.end();
        if (!isFriend && !existInForiegnList){
            unkownAirtagList[deviceName].ts = simTime();
            unkownAirtagList[deviceName].distance = CurrPos.distance(deviceLocation);

            EV_INFO << unkownAirtagList[deviceName].distance << endl;
        }else if (existInForiegnList){
            simtime_t currTS = simTime();
            float currDistance = CurrPos.distance(deviceLocation);
            if(currTS - unkownAirtagList[deviceName].ts > 3 && currDistance < 100.0 && unkownAirtagList[deviceName].distance < 100.0){
                    EV_INFO << "NOTIFICATION: UNKNOWN AIRTAG: " << deviceName << "IS MOVING WITH IPHONE: " << ownName << endl;
                    unkownAirtagList[deviceName].ts = simTime();
                    unkownAirtagList[deviceName].distance = CurrPos.distance(deviceLocation);
                    unkownAirtagFollowCount++;
                    emit(unkownAirtagFollow, unkownAirtagFollowCount);    
            }
        }
        EV_INFO << "IPHONE HOST "<< ownName << ": FORWARDING " << deviceName <<" DATA TO THE ICLOUD" << endl;
        Packet *packet = new Packet("AirTag Location");
        packet->addTag<FragmentationReq>()->setDontFragment(true);

        const auto& payload = makeShared<DataPacket>();
        payload->setChunkLength(B(20));
        payload->setType(0);
        payload->setIp(ip);
        payload->setDeviceName(deviceName.c_str());
        payload->setIsDeviceFound(isDeviceFound);

        payload->setDeviceLocation(deviceLocation);

        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        emit(packetSentSignal, packet);
        socket->sendTo(packet, destAddress, destPort);
        numSent++;
    }else{
        if (packetType == 2) {
            senderHostIdInt = 0;
        }


        if (packetType == 2) {
            if (isDeviceFound == -1) { // AirTag Not Found
                EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". AIRTAG: " << deviceName << " IS NOT IN THE ICLOUD "<< endl;
            }
            else { // Airtag Found
                    EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". AIRTAG: " << deviceName << " IS AT THE LOCATION: " << deviceLocation << endl;
                    int k = intrand(100);
                    if (k < 50) {
                    sendLocationToIphone(deviceName, deviceLocation);
                    }
                }
        }else if (packetType == 5){
            if (isDeviceFound == -1) { // iPhone Not Found
                EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". IPHONE: " << deviceName << " IS NOT IN THE ICLOUD "<< endl;
            }
            else { // iPhone Found
                addressBook[deviceName].ip = ip;
                addressBook[deviceName].ts = simTime();

                EV_INFO << "LOOKUP RESPONSE RECEIVED BY: " << ownName << ". IPHONE: " << deviceName << " IS AT THE LOCATION: " << deviceLocation << endl;
                int k = intrand(100);

                if (k < 50) {
                sendLocationToIphone(deviceName, deviceLocation);
                }

            }      
        }else if (packetType == 6){
            std::string specific_airtag = "airtag[3]";
            auto textMessage = data->getTextMessage();
            EV_INFO << "MESSAGE RECEIVED: " << textMessage << ", RECEIVED DEVICE LOCATION IS:" << deviceLocation << endl;
            friendlyMessagesCount++;
            emit(friendlyMessages, friendlyMessagesCount);  
        }
    }
    delete pk;
}

void ClientApp::finish() {
    recordScalar("chunkLength", chunkLength);
    recordScalar("unkownAirtagFollow", unkownAirtagFollow);
    recordScalar("friendlyMessages", friendlyMessages);
    super::finish();
}

} // namespace inet

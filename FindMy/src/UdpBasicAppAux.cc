#include "UdpBasicAppAux.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

UdpBasicAppAux::~UdpBasicAppAux()
{
    cancelAndDelete(selfMsg);
}

void UdpBasicAppAux::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ownName = std::string(getParentModule()->getFullName());
        startTime = par("startTime");
        stopTime = par("stopTime");

        sendingEnabled = par("sendingEnabled");
        listeningEnabled = par("listeningEnabled");

        if (listeningEnabled) {
            localPort = par("localPort");
        }

        if (stopTime >= CLOCKTIME_ZERO && stopTime < startTime) {
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        }

        selfMsg = new ClockEvent("sendTimer");

        // statistics
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
    }

    if (stage == INITSTAGE_LAST) {
        if (sendingEnabled) {
            destAddress = L3AddressResolver().resolve(par("destAddress"));
            destPort = par("destPort");
        }
    }

    EV_INFO << "UDP-START" << endl;
}

void UdpBasicAppAux::finish()
{
    // statistics
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UdpBasicAppAux::sendPacket() {}

void UdpBasicAppAux::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    if (listeningEnabled) {
        socket.bind(localPort);
    }

    socket.setCallback(this);

    if (sendingEnabled) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= CLOCKTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleClockEventAt(stopTime, selfMsg);
        }
    }
}

void UdpBasicAppAux::processSend()
{
    sendPacket();
    clocktime_t d = par("sendInterval");
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }
}

void UdpBasicAppAux::processStop()
{
    socket.close();
}

void UdpBasicAppAux::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void UdpBasicAppAux::socketDataArrived(UdpSocket *socket, Packet *packet) { }

void UdpBasicAppAux::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpBasicAppAux::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpBasicAppAux::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "---");
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpBasicAppAux::handleStartOperation(LifecycleOperation *operation)
{
    clocktime_t start = std::max(startTime, getClockTime());
    if ((stopTime < CLOCKTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleClockEventAt(start, selfMsg);
    }
}

void UdpBasicAppAux::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpBasicAppAux::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy();
    socket.setCallback(nullptr);
}

} // namespace inet

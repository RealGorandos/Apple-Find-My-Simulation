#ifndef UDPBASICAPPAUX_H_
#define UDPBASICAPPAUX_H_

#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

extern template class ClockUserModuleMixin<ApplicationBase>;

class INET_API UdpBasicAppAux : public ClockUserModuleMixin<ApplicationBase>, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };
    clocktime_t startTime;
    clocktime_t stopTime;

    std::string ownName;

    const char *packetName = nullptr;

    bool sendingEnabled;
    L3Address destAddress;
    int destPort;

    bool listeningEnabled;
    int localPort;

    UdpSocket socket;
    ClockEvent *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void sendPacket();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpBasicAppAux() {}
    ~UdpBasicAppAux();
};

} // namespace inet

#endif /* UDPBASICAPPAUX_H_ */

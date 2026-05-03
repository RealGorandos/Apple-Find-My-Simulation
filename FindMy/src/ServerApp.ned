package FindMy;

import inet.applications.contract.IApp;
@namespace(inet);
simple ServerApp like IApp
{
    parameters:
        
        string clockModule = default(""); // relative path of a module that implements IClock; optional

        bool sendingEnabled = default(false);
        
        bool listeningEnabled = default(true);
        int localPort = default(4444);
        
        double startTime @unit(s) = default(this.sendInterval); // application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        volatile double sendInterval @unit(s) = default(1s); // should usually be a random value, e.g. exponential(1)

        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        
        @signal[successfulLookupAirTag](type="long");
        @statistic[successfulLookupAirTag](title="Successful AirTag Lookup Count"; source="successfulLookupAirTag"; record=vector; interpolationmode=linear);
        
        @signal[unsuccessfulLookupAirTag](type="long");
        @statistic[unsuccessfulLookupAirTag](title="Unsuccessful AirTag Lookup Count"; source="unsuccessfulLookupAirTag"; record=vector; interpolationmode=linear);
        
        @signal[successfulLookupIphone](type="long");
        @statistic[successfulLookupIphone](title="Successful Iphone Lookup Count"; source="successfulLookupIphone"; record=vector; interpolationmode=linear);
        
        @signal[unsuccessfulLookupIphone](type="long");
        @statistic[unsuccessfulLookupIphone](title="Unsuccessful Iphone Lookup Count"; source="unsuccessfulLookupIphone"; record=vector; interpolationmode=linear);
        
        @signal[endtoenddelay](type="long");
        @statistic[endtoenddelay](title="End-to-end Delay"; source="endtoenddelay"; record=vector; interpolationmode=linear);
        
        
        @signal[senderHostId](type="long");
        @statistic[senderHostId](title="Sender Host ID"; source="senderHostId"; record=vector; interpolationmode=linear);
        
        
        @signal[receiverHostId](type="long");
        @statistic[receiverHostId](title="Receiver Host ID"; source="receiverHostId"; record=vector; interpolationmode=linear);
        
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[throughput](title="throughput"; unit=bps; source="throughput(packetSent)"; record=vector);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="dataAge(packetReceived)"; unit=s; record=stats,vector; interpolationmode=none);
        @statistic[rcvdPkSeqNo](title="received packet sequence number"; source="appPkSeqNo(packetReceived)"; record=vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
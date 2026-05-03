package FindMy;

import inet.applications.contract.IApp;
@namespace(inet);
simple ClientApp like IApp
{
    parameters:    
        int airtag_number;
        int apple_host_number;
        int friendly_iphones_num;
        int friendly_airtags_num;
        string airtag_prefix = default("airtag");
        string apple_device_prefix = default("apple_device_host");
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        
        bool sendingEnabled = default(true);
        string destAddress = default("");
        int destPort = default(4444);

        bool listeningEnabled = default(true);
        int localPort = default(5555);

        double startTime @unit(s) = default(this.sendInterval); // application start time (start of the first packet)
        double stopTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        volatile double sendInterval @unit(s); // should usually be a random value, e.g. exponential(1)

        @display("i=block/app");
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        
        @signal[chunkLength](type="long");
        @statistic[chunkLength](title="Chunk Length"; source="chunkLength"; record=vector; interpolationmode=none);
        
        @signal[endtoenddelay](type="long");
        @statistic[endtoenddelay](title="End-to-end Delay"; source="endtoenddelay"; record=vector; interpolationmode=linear);
        
        @signal[unkownAirtagFollow](type="long");
        @statistic[unkownAirtagFollow](title="Amount of times an Iphone is being followed by an unkown AirTag Count"; source="unkownAirtagFollow"; record=vector; interpolationmode=linear);

        @signal[friendlyMessages](type="long");
        @statistic[friendlyMessages](title="Amount of times an Iphone received a friendly message"; source="friendlyMessages"; record=vector; interpolationmode=linear);
      
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
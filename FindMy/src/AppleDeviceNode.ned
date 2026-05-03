//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package FindMy;
import inet.node.base.ApplicationLayerNodeBase;

import inet.applications.contract.IApp;
import inet.common.packet.recorder.PcapRecorder;
import inet.mobility.contract.IMobility;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.ILoopbackInterface;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4RoutingTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.common.packet.recorder.PcapRecorder;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;
import inet.transportlayer.contract.ISctp;
import inet.common.MessageDispatcher;
import simu5g.stack.ICellularNic;

import simu5g.corenetwork.statsCollector.UeStatsCollector;

import simu5g.stack.NRNicUe;

import simu5g.corenetwork.statsCollector.NRueStatsCollector;

module AppleDeviceNode extends ApplicationLayerNodeBase
{
    parameters:
        @networkNode();
        @display("i=device/pocketpc;bgb=858.888,659.568");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=600,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=845,11; anchor=ne; text="application layer");
        @figure[transportLayer](type=rectangle; pos=250,156; size=600,130; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=845,161; anchor=ne; text="transport layer");
        @figure[networkLayer](type=rectangle; pos=250,306; size=600,130; fillColor=#00ff00; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[networkLayer.title](type=text; pos=845,311; anchor=ne; text="network layer");
        @figure[linkLayer](type=rectangle; pos=250,456; size=600,130; fillColor=#0000ff; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[linkLayer.title](type=text; pos=845,461; anchor=ne; text="link layer");
        numWlanInterfaces = default(1);
        energyStorage.typename = default("IdealEpEnergyStorage");
        wlan[*].typename = default("Ieee802154NarrowbandInterface");
        wlan[*].radio.energyConsumer.typename = default("SensorStateBasedEpEnergyConsumer"); //FIXME does this consumer exist???
        @figure[submodules];

        int nrMasterId @mutable = default(0);
        int nrMacNodeId @mutable = default(0); // TODO: this is not a real parameter
        int nrMacCellId @mutable = default(0); // TODO: this is not a real parameter

        //# Mobility
        mobility.typename = default("StationaryMobility");



        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId @mutable;
        int macNodeId @mutable = default(0); // TODO: this is not a real parameter
        int macCellId @mutable = default(0); // TODO: this is not a real parameter

        //# D2D capability
        string nicType = default("NRNicUe");


        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^.ipv4.routingTable");

        //# External Ethernet interfaces
        string extHostAddress = default("");

        //# Loopback interfaces

        //# Num of PcapRecorders

        bool hasRNISupport = default(false);

    gates:
        input nrRadioIn @directIn;     // connection to master  

    submodules:



        //# layer submodules (must be ordered from lower to upper layers)

        //# link layer 

        cellularNic: <nicType> like ICellularNic {
            nodeType = parent.nodeType;
            @display("p=355.152,589.50397");
        }

        //# ethernet interface used for communicating with external applications in emulation mode
        encap: <default("EthernetEncapsulation")> like IEthernetLayer if typename != "" {
            parameters:
                registerProtocol = true;
                @display("p=674.06396,496.488");
        }



        //# app layer 


        //# message dispatcher for SAP between application and transport layer

        //# message dispatcher for SAP between transport and network layer

        //# message dispatcher for SAP to link layer

        //# PcapRecorders


        //# UeStatsCollector - for MEC
        ueCollector: UeStatsCollector if hasRNISupport {
            @display("p=126.84,588.296;is=s");
        }

    connections allowunconnected:

        //# 
        //# Internal Tcp/Udp applications connections with transport layer
        //#

//        for i=0..numApps-1 {
//            app[i].socketOut --> at.in++;
//            app[i].socketIn <-- at.out++;
//        }

//        at.out++ --> udp.appIn if hasUdp;
//        at.in++ <-- udp.appOut if hasUdp;
//
//        at.out++ --> tcp.appIn if hasTcp;
//        at.in++ <-- tcp.appOut if hasTcp;
//
//        at.out++ --> sctp.appIn if hasSctp;
//        at.in++ <-- sctp.appOut if hasSctp;

        //# 
		//# Internal transport layer connections to network layer
		//# 

//        udp.ipOut --> tn.in++ if hasUdp;
//        udp.ipIn <-- tn.out++ if hasUdp;
//
//        tcp.ipOut --> tn.in++ if hasTcp;
//        tcp.ipIn <-- tn.out++ if hasTcp;
//
//        sctp.ipOut --> tn.in++ if hasSctp;
//        tn.out++ --> sctp.ipIn if hasSctp;

//        ipv4.ifIn <-- nl.out++ if hasIpv4;
//        ipv4.ifOut --> nl.in++ if hasIpv4;
//
//        ipv4.transportIn <-- tn.out++ if hasIpv4;
//        ipv4.transportOut --> tn.in++ if hasIpv4;
//
//        ipv6.ifIn <-- nl.out++ if hasIpv6;
//        ipv6.ifOut --> nl.in++ if hasIpv6;
//
//        ipv6.transportIn <-- tn.out++ if hasIpv6;
//        ipv6.transportOut --> tn.in++ if hasIpv6;
//
//        tn.out++ --> nl.in++;
//        tn.in++ <-- nl.out++;

        //#
        //# Internal link layer connections to network layer  
        //#

        cellularNic.upperLayerOut --> nl.in++;
        cellularNic.upperLayerIn <-- nl.out++;

        for i=0..numEthInterfaces-1 {
            encap.lowerLayerOut --> eth[i].upperLayerIn;
            encap.lowerLayerIn <-- eth[i].upperLayerOut;
        }
        encap.upperLayerOut --> nl.in++ if numEthInterfaces != 0;
        encap.upperLayerIn <-- nl.out++ if numEthInterfaces != 0;

//        for i=0..numLoInterfaces-1 {
//            nl.out++ --> lo[i].upperLayerIn;
//            lo[i].upperLayerOut --> nl.in++;
//        }

        //#
        //# Connections between LTE/NR NIC and the radio medium
        //#
        cellularNic.nrRadioIn <-- nrRadioIn;
}
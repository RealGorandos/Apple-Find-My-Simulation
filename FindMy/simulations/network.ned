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

package FindMy.simulations;


import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import ned.DatarateChannel;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;

import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.world.radio.LteChannelControl;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.Upf;
import FindMy.AppleDeviceNode;

network Network
{
    parameters:
        int apple_host_number;
        int airtag_number;
        double cableLength @unit(m);
        double cableBandwidth @unit(bps);
        double closeCableLength @unit(m);
        double closeCableBandwidth @unit(bps);
        @display("bgb=5000,5000");

    types:
        channel cable extends DatarateChannel
        {
            delay = replaceUnit(parent.cableLength / 2e8, "s");
            datarate = parent.cableBandwidth;
        }

        channel closeCable extends DatarateChannel
        {
            delay = replaceUnit(parent.closeCableLength / 2e8, "s");
            datarate = parent.closeCableBandwidth;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=0,0");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=0,750");
        }
        channelControl: LteChannelControl {
            @display("p=0,1500");
        }
        binder: Binder {
            @display("p=0,2250");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=0,3000");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=0,3750");
        }
        // =====[ Main ]======
        mainrouter: Router {
            @display("p=2030.9725,159.65598");
        }
        server: StandardHost {
            @display("p=1477.746,159.65598");
        }
        upf: Upf {
            @display("p=2030.9725,471.54205");
        }
        iUpf1: Upf {
            @display("p=2022,1421");
        }

        // =====[ gnb ]======
        gnb1: gNodeB {
            @display("p=1071,4083");
        }
        gnb2: gNodeB {
            @display("p=2352,2812");
        }
        gnb3: gNodeB {
            @display("p=3673,4073");
        }

        // =====[ iPhone ]======
        apple_device_host[apple_host_number]: AppleDeviceNode {
            parameters:
                @display("p=240,475;i=Apple_Logo");
        }
        // =====[ AirTag ]======
        airtag[airtag_number]: SensorNode {
            @display("p=610,1621;i=Apple_AirTag");
        }
    connections:
        server.pppg++ <--> closeCable <--> mainrouter.pppg++;
        upf.filterGate <--> closeCable <--> mainrouter.pppg++;

        upf.pppg++ <--> cable <--> iUpf1.pppg++;


        gnb1.ppp <--> closeCable <--> iUpf1.pppg++;
        gnb2.ppp <--> closeCable <--> iUpf1.pppg++;
        gnb3.ppp <--> closeCable <--> iUpf1.pppg++;

		//# X2 connections
        gnb1.x2++ <--> closeCable <--> gnb2.x2++;
        gnb2.x2++ <--> closeCable <--> gnb3.x2++;
        gnb3.x2++ <--> closeCable <--> gnb1.x2++;

}

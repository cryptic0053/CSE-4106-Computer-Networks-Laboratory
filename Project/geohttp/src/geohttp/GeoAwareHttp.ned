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

package geohttp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EtherSwitch;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ethernet.common.EtherQoSQueue;
import inet.linklayer.contract.IEthernetInterface;
import inet.common.misc.ThruputMeteringChannel;

network GeoAwareHttp
{
    parameters:
        int numClients = default(60);
        int numMirrors = default(4);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,50");
        }
        access1: EtherSwitch { @display("p=250,200"); }
        access2: EtherSwitch { @display("p=250,380"); }
        core: EtherSwitch { @display("p=520,290"); }

        lb: StandardHost {
            parameters:
                @display("p=720,210");
                numApps = 1;
                app[0].typename = "GeoLoadBalancerApp";
                app[0].localPort = 8080;
                app[0].mode = default("302"); // "302" or "proxy" (TODO)
                app[0].registryAddress = default("registry");
                app[0].registryPort = default(9090);
                app[0].weightDistance = default(0.7);
                app[0].weightLoad = default(0.3);
                app[0].downPenalty = default(10);
                app[0].registryExpiryMs = default(5000);
        }

        registry: StandardHost {
            parameters:
                @display("p=720,380");
                numApps = 1;
                app[0].typename = "MirrorRegistryApp";
                app[0].localPort = 9090;
                app[0].expiryMs = default(6000);
        }

        client[numClients]: StandardHost {
            parameters:
                @display("p=90,260,row,50;i=block/app");
                numApps = 1;
                app[0].typename = "GeoClientApp";
                app[0].connectAddress = "lb";
                app[0].connectPort = 8080;
                app[0].requestInterval = exponential(1s);
                app[0].objectSizeBytes = intuniform(20kB, 100kB);
                app[0].geoLat = normal(23.8deg, 2deg);
                app[0].geoLon = normal(90.4deg, 2deg);
        }

        mirror[numMirrors]: StandardHost {
            parameters:
                @display("p=980,290,row,60;i=block/server");
                numApps = 2;
                app[0].typename = "MirrorServerApp";
                app[0].localPort = 8081;
                app[0].serviceRateBps = 50Mbps;  // emulate load
                app[0].mirrorId = stringFormat("m%d", index);
                app[0].geoLat = default(0deg);
                app[0].geoLon = default(0deg);

                app[1].typename = "MirrorReporterApp"; // reports to registry
                app[1].reportInterval = exponential(1s);
                app[1].registryAddress = "registry";
                app[1].registryPort = 9090;
                app[1].mirrorId = stringFormat("m%d", index);
                app[1].geoLat = default(0deg);
                app[1].geoLon = default(0deg);
        }
    connections allowunconnected:
        // access to core
        access1.ethg++ <--> ThruputMeteringChannel { delay = 5ms; } <--> core.ethg++;
        access2.ethg++ <--> ThruputMeteringChannel { delay = 10ms; } <--> core.ethg++;

        // lb & registry to core
        lb.ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> core.ethg++;
        registry.ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> core.ethg++;

        // clients split across access switches
        for i=0..numClients-1 {
            client[i].ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> (i%2==0 ? access1.ethg++ : access2.ethg++);
        }

        // mirrors to core (simulate geography by delay)
        for j=0..numMirrors-1 {
            mirror[j].ethg++ <--> ThruputMeteringChannel { delay = 30ms + j*40ms; } <--> core.ethg++;
        }
}


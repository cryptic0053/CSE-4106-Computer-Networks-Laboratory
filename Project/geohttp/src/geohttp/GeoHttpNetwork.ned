package geohttp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ethernet.EthernetSwitch;
import inet.common.misc.ThruputMeteringChannel;

network GeoHttpNetwork
{
    parameters:
        int numClients = default(6);
        int numMirrors = default(3);

    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,50");
        }

        access1: EthernetSwitch { @display("p=250,200"); }
        access2: EthernetSwitch { @display("p=250,380"); }
        core:    EthernetSwitch { @display("p=520,290"); }

        lb: StandardHost {
            parameters:
                @display("p=720,210");
        }

        registry: StandardHost {
            parameters:
                @display("p=720,380");
        }

        client[numClients]: StandardHost {
            parameters:
                @display("p=90,260,row,50;i=block/app");
        }

        mirror[numMirrors]: StandardHost {
            parameters:
                @display("p=980,290,row,60;i=block/server");
        }

    connections allowunconnected:
        // access <-> core
        access1.ethg++ <--> ThruputMeteringChannel { delay = 5ms; }  <--> core.ethg++;
        access2.ethg++ <--> ThruputMeteringChannel { delay = 10ms; } <--> core.ethg++;

        // lb & registry <-> core
        lb.ethg++       <--> ThruputMeteringChannel { delay = 1ms; } <--> core.ethg++;
        registry.ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> core.ethg++;

        // Connect all clients to their respective access switches
        for i=0..int(numClients/2)-1 {
            client[i*2].ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> access1.ethg++;
        }
        for i=0..int((numClients-1)/2) {
            client[i*2+1].ethg++ <--> ThruputMeteringChannel { delay = 1ms; } <--> access2.ethg++;
        }

        // mirrors to core (emulate geography with delay gradient)
        for j=0..numMirrors-1 {
            mirror[j].ethg++ <--> ThruputMeteringChannel { delay = 30ms + j * 40ms; } <--> core.ethg++;
        }
}

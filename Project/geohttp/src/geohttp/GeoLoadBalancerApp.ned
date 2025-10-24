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

simple GeoLoadBalancerApp like inet.applications.tcpapp.TcpAppBase
{
    parameters:
        int localPort;
        string mode; // "302" or "proxy" (proxy TODO)
        string registryAddress;
        int registryPort;
        double weightDistance;
        double weightLoad;
        double downPenalty;
        int registryExpiryMs;
        @display("i=block/app3");
    gates:
        input socketIn;
        output socketOut;
}


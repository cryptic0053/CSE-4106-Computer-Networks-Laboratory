package geohttp;

simple GeoLoadBalancerApp like inet.applications.contract.IApp
{
    parameters:
        int localPort = default(8080);
        string mode = default("302");           // "302" or "proxy"
        string registryAddress = default("registry");
        int registryPort = default(9090);
        double weightDistance = default(0.7);
        double weightLoad = default(0.3);
        double downPenalty = default(10);
        int registryExpiryMs = default(5000);
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}

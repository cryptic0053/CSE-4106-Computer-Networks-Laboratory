package geohttp;

simple MirrorRegistryApp like inet.applications.contract.IApp
{
    parameters:
        int localPort = default(9090);
        int expiryMs = default(6000);
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}

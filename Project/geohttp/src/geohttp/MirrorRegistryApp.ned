package geohttp;

simple MirrorRegistryApp like inet.applications.contract.IApp
{
    parameters:
        // Standard IApp parameters (required by INET)
        string localAddress = default("");
        int localPort = default(9090);
        bool autoRead = default(true);

        // MirrorRegistryApp specific parameters
        int expiryMs = default(6000);
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
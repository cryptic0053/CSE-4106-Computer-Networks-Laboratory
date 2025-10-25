package geohttp;

simple MirrorServerApp like inet.applications.contract.IApp
{
    parameters:
        // Standard IApp parameters (required by INET)
        string localAddress = default("");
        int localPort = default(8081);
        bool autoRead = default(true);

        // MirrorServerApp specific parameters
        string mirrorId = default("");
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}
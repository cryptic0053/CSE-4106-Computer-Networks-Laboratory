package geohttp;

simple MirrorReporterApp like inet.applications.contract.IApp
{
    parameters:
        // Standard IApp parameters (required by INET)
        string localAddress = default("");
        int localPort = default(-1);
        bool autoRead = default(true);

        // MirrorReporterApp specific parameters
        string registryAddress;
        int registryPort;
        string mirrorId;
        double latitude = default(0);   // in degrees
        double longitude = default(0);  // in degrees
        double load = default(0.3);
        bool ok = default(true);
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}
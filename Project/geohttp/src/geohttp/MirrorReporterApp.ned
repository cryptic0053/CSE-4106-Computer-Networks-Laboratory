package geohttp;

simple MirrorReporterApp like inet.applications.contract.IApp
{
    parameters:
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

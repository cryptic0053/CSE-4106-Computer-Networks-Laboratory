package geohttp;

simple GeoClientApp like inet.applications.contract.IApp
{
    parameters:
        // Standard IApp parameters (required by INET)
        string localAddress = default("");
        int localPort = default(-1);
        bool autoRead = default(true);

        // GeoClientApp specific parameters
        string connectAddress;
        int connectPort;
        double latitude = default(0);   // in degrees
        double longitude = default(0);  // in degrees
        string path = default("/");
        @display("i=block/app");
    gates:
        input socketIn;
        output socketOut;
}
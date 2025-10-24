package geohttp;

simple GeoClientApp like inet.applications.contract.IApp
{
    parameters:
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

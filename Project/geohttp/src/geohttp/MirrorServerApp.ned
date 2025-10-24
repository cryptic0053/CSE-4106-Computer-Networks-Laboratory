package geohttp;

simple MirrorServerApp like inet.applications.contract.IApp
{
    parameters:
        int localPort = default(8081);
        string mirrorId = default("");
        @display("i=block/server");
    gates:
        input socketIn;
        output socketOut;
}

#include "GeoClientApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

using namespace inet;

Define_Module(GeoClientApp);

void GeoClientApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        connectAddressStr = par("connectAddress").stdstringValue();
        connectPort = par("connectPort");
        if (hasPar("latitude"))  latitude  = par("latitude");
        if (hasPar("longitude")) longitude = par("longitude");
        if (hasPar("path"))      path      = par("path").stdstringValue();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        L3AddressResolver().tryResolve(connectAddressStr.c_str(), connectAddress);

        socket = new TcpSocket();
        socket->setOutputGate(gate("socketOut"));
        socket->setCallback(this);
        socket->connect(connectAddress, connectPort);
    }
}

void GeoClientApp::handleMessageWhenUp(cMessage *msg)
{
    if (socket && socket->belongsToSocket(msg)) {
        socket->processMessage(msg);
    }
    else {
        delete msg;
    }
}

void GeoClientApp::socketEstablished(TcpSocket *s)
{
    EV_INFO << "Client connected; sending HTTP GET with X-Client-Geo\n";
    sendRequest();
}

void GeoClientApp::sendRequest()
{
    std::ostringstream oss;
    oss << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << connectAddressStr << ":" << connectPort << "\r\n"
        << "X-Client-Geo: " << latitude << "," << longitude << "\r\n"
        << "Connection: close\r\n\r\n";

    std::string req = oss.str();
    std::vector<uint8_t> data(req.begin(), req.end());
    auto payload = makeShared<BytesChunk>(data);

    auto pkt = new Packet("http-get");
    pkt->insertAtBack(payload);
    socket->send(pkt);
}

void GeoClientApp::socketDataArrived(TcpSocket *, Packet *pkt, bool)
{
    // We don't parse the body here, just discard
    delete pkt;
}

void GeoClientApp::socketClosed(TcpSocket *s)
{
    delete s;
    socket = nullptr;
}

void GeoClientApp::socketFailure(TcpSocket *s, int code)
{
    EV_WARN << "Client socket failure code=" << code << "\n";
    delete s;
    socket = nullptr;
}

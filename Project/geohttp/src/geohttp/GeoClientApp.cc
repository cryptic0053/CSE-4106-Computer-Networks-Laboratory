#include "GeoClientApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/INETUtils.h"

Define_Module(GeoClientApp);

void GeoClientApp::initialize(int stage) {
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        connectPort = par("connectPort");
        geoLat = par("geoLat").doubleValue();
        geoLon = par("geoLon").doubleValue();
        nextReqInterval = par("requestInterval").doubleValue();
        objectSize = par("objectSizeBytes").intValue();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        L3AddressResolver().tryResolve(par("connectAddress"), connectAddress);
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        socket.connect(connectAddress, connectPort);
    }
}

void GeoClientApp::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        sendRequest();
    } else {
        socket.processMessage(msg);
    }
}

void GeoClientApp::socketEstablished(TcpSocket *socket) {
    connected = true;
    scheduleAt(simTime() + nextReqInterval, new cMessage("send"));
}

void GeoClientApp::sendRequest() {
    if (!connected) return;

    // Minimal HTTP GET with custom header
    std::ostringstream oss;
    oss << "GET /object?size=" << objectSize << " HTTP/1.1\r\n";
    oss.setf(std::ios::fixed); oss.precision(6);
    oss << "Host: lb\r\n";
    oss << "X-Client-Geo: " << geoLat << "," << geoLon << "\r\n";
    oss << "Connection: keep-alive\r\n\r\n";

    auto payload = makeShared<BytesChunk>(oss.str());
    auto pkt = new Packet("http-get");
    pkt->insertAtBack(payload);
    socket.send(pkt);

    // schedule next
    nextReqInterval = par("requestInterval").doubleValue();
    scheduleAt(simTime() + nextReqInterval, new cMessage("send"));
}

void GeoClientApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool) {
    // we don't parse deeply; just consume the response
    delete pkt;
}

void GeoClientApp::socketClosed(TcpSocket *socket) { connected = false; }
void GeoClientApp::socketFailure(TcpSocket *socket, int) { connected = false; }

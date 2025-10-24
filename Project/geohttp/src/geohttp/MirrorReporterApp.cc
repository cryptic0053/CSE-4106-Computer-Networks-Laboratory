#include "MirrorReporterApp.h"

Define_Module(MirrorReporterApp);

void MirrorReporterApp::initialize(int stage) {
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registryPort = par("registryPort");
        mirrorId = par("mirrorId").stdstringValue();
        geoLat = par("geoLat").doubleValue();
        geoLon = par("geoLon").doubleValue();
        reportInterval = par("reportInterval").doubleValue();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        L3AddressResolver().tryResolve(par("registryAddress"), registryAddr);
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
        connectIfNeeded();
        scheduleAt(simTime() + reportInterval, new cMessage("tick"));
    }
}

void MirrorReporterApp::handleMessageWhenUp(cMessage *msg) {
    if (msg->isSelfMessage()) {
        connectIfNeeded();
        sendReport();
        scheduleAt(simTime() + reportInterval, msg);
    } else {
        socket.processMessage(msg);
    }
}

void MirrorReporterApp::connectIfNeeded() {
    if (!connected)
        socket.connect(registryAddr, registryPort);
}

void MirrorReporterApp::sendReport() {
    if (!connected) return;
    // Simple line protocol: REPORT id=... lat=... lon=... load=... ok=1
    // Here we don't compute live load inside mirror; you can wire a signal or param if needed.
    double fakeLoad = uniform(0, 1); // demo; replace with your queue depth or req/s
    std::ostringstream oss;
    oss.setf(std::ios::fixed); oss.precision(6);
    oss << "REPORT id=" << mirrorId << " lat=" << geoLat << " lon=" << geoLon
        << " load=" << fakeLoad << " ok=1\n";
    auto pkt = new Packet("report");
    pkt->insertAtBack(makeShared<BytesChunk>(oss.str()));
    socket.send(pkt);
}

void MirrorReporterApp::socketEstablished(TcpSocket *) { connected = true; }
void MirrorReporterApp::socketDataArrived(TcpSocket *, Packet *pkt, bool) { delete pkt; }
void MirrorReporterApp::socketClosed(TcpSocket *) { connected = false; }
void MirrorReporterApp::socketFailure(TcpSocket *, int) { connected = false; }

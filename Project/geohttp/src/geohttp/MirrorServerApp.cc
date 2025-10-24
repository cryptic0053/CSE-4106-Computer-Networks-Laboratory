#include "MirrorServerApp.h"
#include <regex>

Define_Module(MirrorServerApp);

void MirrorServerApp::initialize(int stage) {
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        mirrorId = par("mirrorId").stdstringValue();
        geoLat = par("geoLat").doubleValue();
        geoLon = par("geoLon").doubleValue();
        serviceRateBps = par("serviceRateBps").doubleValue();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        TcpSocket *listener = new TcpSocket();
        listener->setOutputGate(gate("socketOut"));
        listener->setCallback(this);
        listener->bind(localPort);
        listener->listen();
        socketMap.addSocket(listener);
    }
}

void MirrorServerApp::handleMessageWhenUp(cMessage *msg) {
    auto ind = dynamic_cast<TcpAvailableInfo *>(msg->getControlInfo());
    if (ind) {
        auto *socket = new TcpSocket(ind);
        socket->setOutputGate(gate("socketOut"));
        socket->setCallback(this);
        socketMap.addSocket(socket);
    } else {
        TcpSocket *sock = socketMap.findSocketFor(msg);
        if (sock) sock->processMessage(msg);
        else delete msg;
    }
}

void MirrorServerApp::socketAvailable(TcpSocket *, TcpAvailableInfo *) { }

void MirrorServerApp::socketEstablished(TcpSocket *) { }

void MirrorServerApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool) {
    // Parse object size from GET /object?size=NNN
    auto str = pkt->peekAtFront<BytesChunk>()->getBytes().str();
    delete pkt;

    int sizeBytes = 50000;
    std::smatch m;
    if (std::regex_search(str, m, std::regex("GET /object\\?size=(\\d+)")))
        sizeBytes = std::stoi(m[1]);

    replyOk(socket, sizeBytes);
}

void MirrorServerApp::replyOk(TcpSocket *socket, int objectBytes) {
    // emulate service time
    double seconds = objectBytes * 8.0 / serviceRateBps;
    scheduleAt(simTime() + seconds, new cMessage("done")); // lightweight; not per-conn accurate

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Length: " << objectBytes << "\r\n"
        << "X-Mirror-Id: " << mirrorId << "\r\n\r\n";
    auto pkt = new Packet("http-200");
    pkt->insertAtBack(makeShared<BytesChunk>(oss.str()));
    socket->send(pkt);
}

void MirrorServerApp::socketClosed(TcpSocket *) { }
void MirrorServerApp::socketFailure(TcpSocket *, int) { }

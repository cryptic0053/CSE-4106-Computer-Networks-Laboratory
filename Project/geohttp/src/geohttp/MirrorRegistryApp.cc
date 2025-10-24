#include "MirrorRegistryApp.h"
#include <regex>

Define_Module(MirrorRegistryApp);

void MirrorRegistryApp::initialize(int stage) {
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        expiryMs = par("expiryMs");
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

void MirrorRegistryApp::handleMessageWhenUp(cMessage *msg) {
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

void MirrorRegistryApp::socketAvailable(TcpSocket *, TcpAvailableInfo *) { }
void MirrorRegistryApp::socketEstablished(TcpSocket *) { }

void MirrorRegistryApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool) {
    auto s = pkt->peekAtFront<BytesChunk>()->getBytes().str();
    delete pkt;

    if (s.rfind("REPORT", 0) == 0) {
        handleReport(s);
    } else if (s.rfind("GET /mirrors", 0) == 0) {
        handleList(socket);
    }
}

void MirrorRegistryApp::handleReport(const std::string& line) {
    // REPORT id=mx lat=.. lon=.. load=.. ok=0/1
    std::regex kvre("(id|lat|lon|load|ok)=([^\\s]+)");
    std::smatch m;
    MirrorInfo info;
    info.lastSeen = simTime();
    for (auto i = std::sregex_iterator(line.begin(), line.end(), kvre); i != std::sregex_iterator(); ++i) {
        auto k = (*i)[1].str(), v = (*i)[2].str();
        if (k == "id") info.id = v;
        else if (k == "lat") info.lat = std::stod(v);
        else if (k == "lon") info.lon = std::stod(v);
        else if (k == "load") info.load = std::stod(v);
        else if (k == "ok") info.ok = (v == "1");
    }
    if (!info.id.empty()) db[info.id] = info;
}

void MirrorRegistryApp::handleList(TcpSocket *socket) {
    // Return simple JSON lines (healthy + not expired)
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n[";
    bool first = true;
    for (auto &kv : db) {
        auto mi = kv.second;
        if (!mi.ok) continue;
        if ((simTime() - mi.lastSeen).inUnit(SIMTIME_MS) > expiryMs) continue;
        if (!first) oss << ",";
        first = false;
        oss.setf(std::ios::fixed); oss.precision(6);
        oss << "{\"id\":\"" << mi.id << "\",\"lat\":" << mi.lat << ",\"lon\":" << mi.lon
            << ",\"load\":" << mi.load << "}";
    }
    oss << "]";
    auto pkt = new Packet("mirrors");
    pkt->insertAtBack(makeShared<BytesChunk>(oss.str()));
    socket->send(pkt);
}

void MirrorRegistryApp::socketClosed(TcpSocket *) { }
void MirrorRegistryApp::socketFailure(TcpSocket *, int) { }

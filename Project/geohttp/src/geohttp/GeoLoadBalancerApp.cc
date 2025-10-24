#include "GeoLoadBalancerApp.h"
#include <regex>
#include "inet/common/packet/Packet.h"

Define_Module(GeoLoadBalancerApp);

void GeoLoadBalancerApp::initialize(int stage) {
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        mode = par("mode").stdstringValue();
        wDist = par("weightDistance").doubleValue();
        wLoad = par("weightLoad").doubleValue();
        wDown = par("downPenalty").doubleValue();
        expiryMs = par("registryExpiryMs");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        TcpSocket *listener = new TcpSocket();
        listener->setOutputGate(gate("socketOut"));
        listener->setCallback(this);
        listener->bind(localPort);
        listener->listen();
        socketMap.addSocket(listener);

        L3AddressResolver().tryResolve(par("registryAddress"), registryAddr);
        registryPort = par("registryPort");
    }
}

void GeoLoadBalancerApp::handleMessageWhenUp(cMessage *msg) {
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

void GeoLoadBalancerApp::socketAvailable(TcpSocket *, TcpAvailableInfo *) { }
void GeoLoadBalancerApp::socketEstablished(TcpSocket *) { }

void GeoLoadBalancerApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool) {
    auto req = pkt->peekAtFront<BytesChunk>()->getBytes().str();
    delete pkt;

    // Parse path and X-Client-Geo header
    std::smatch m;
    std::string path = "/";
    if (std::regex_search(req, m, std::regex("GET ([^\\s]+) HTTP/1")))
        path = m[1];

    double clat = 0, clon = 0;
    if (std::regex_search(req, m, std::regex("X-Client-Geo:\\s*([-0-9\\.]+),([-0-9\\.]+)")))
    {
        clat = std::stod(m[1]); clon = std::stod(m[2]);
    }

    ensureRegistryCache();
    auto chosen = chooseMirror(clat, clon);

    reply302(socket, chosen, path);
}

void GeoLoadBalancerApp::reply302(TcpSocket *socket, const MirrorRow& m, const std::string& path) {
    // Simulate DNS/host names as mirror ID
    std::ostringstream oss;
    oss << "HTTP/1.1 302 Found\r\n"
        << "Location: http://" << m.id << ":8081" << path << "\r\n"
        << "X-Chosen-Mirror: " << m.id << "\r\n\r\n";
    auto pkt = new Packet("http-302");
    pkt->insertAtBack(makeShared<BytesChunk>(oss.str()));
    socket->send(pkt);
}

void GeoLoadBalancerApp::socketClosed(TcpSocket *) { }
void GeoLoadBalancerApp::socketFailure(TcpSocket *, int) { }

void GeoLoadBalancerApp::ensureRegistryCache() {
    if (simTime() - lastFetch < 0.5) return;
    fetchMirrors();
}

void GeoLoadBalancerApp::fetchMirrors() {
    // open a transient socket, GET /mirrors, read JSON array [{"id":"m0","lat":..,"lon":..,"load":..}, ...]
    TcpSocket s; s.setOutputGate(gate("socketOut"));
    s.connect(registryAddr, registryPort);
    // send GET
    {
        auto pkt = new Packet("getmirrors");
        std::string q = "GET /mirrors HTTP/1.1\r\nHost: registry\r\n\r\n";
        pkt->insertAtBack(makeShared<BytesChunk>(q));
        s.send(pkt);
    }
    // RECEIVE one packet (blocking not available; hack: poll via self messages is complex)
    // For a minimal skeleton, we assume immediate arrival handled in the same event loop:
    // In practice, refactor to an async socket with a mini-state machine. For brevity, we “optimistically” read any queued message now.
    cMessage *msg = nullptr;
    while ((msg = s.pullMessage()) != nullptr) {
        auto pkt = check_and_cast<Packet*>(msg);
        auto body = pkt->peekAtFront<BytesChunk>()->getBytes().str();
        // parse JSON very simply
        mirrors.clear();
        std::regex rowRe("\\{\"id\":\"([^\"]+)\",\"lat\":([-0-9\\.]+),\"lon\":([-0-9\\.]+),\"load\":([-0-9\\.]+)\\}");
        for (auto it = std::sregex_iterator(body.begin(), body.end(), rowRe); it != std::sregex_iterator(); ++it) {
            MirrorRow r;
            r.id = (*it)[1].str();
            r.lat = std::stod((*it)[2].str());
            r.lon = std::stod((*it)[3].str());
            r.load = std::stod((*it)[4].str());
            mirrors.push_back(r);
        }
        delete pkt;
    }
    s.close();
    lastFetch = simTime();
}

MirrorRow GeoLoadBalancerApp::chooseMirror(double clat, double clon) {
    if (mirrors.empty()) return MirrorRow{ "fallback", 0, 0, 1.0 };

    // normalize load
    double maxLoad = 0.0001;
    for (auto &m : mirrors) maxLoad = std::max(maxLoad, m.load);

    double bestScore = 1e100;
    MirrorRow best = mirrors.front();

    for (auto &m : mirrors) {
        double dkm = haversine_km(clat, clon, m.lat, m.lon);
        double dnorm = dkm / 20000.0;
        double lnorm = m.load / maxLoad;
        double score = wDist*dnorm + wLoad*lnorm; // health handled upstream
        if (score < bestScore) { bestScore = score; best = m; }
    }
    return best;
}

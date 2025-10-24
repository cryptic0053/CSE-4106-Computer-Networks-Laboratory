#include "GeoLoadBalancerApp.h"
#include "GeoMath.h"
#include <regex>
#include <sstream>
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

using namespace inet;

Define_Module(GeoLoadBalancerApp);

void GeoLoadBalancerApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort   = par("localPort");
        mode        = par("mode").stdstringValue();
        wDist       = par("weightDistance").doubleValue();
        wLoad       = par("weightLoad").doubleValue();
        wDown       = par("downPenalty").doubleValue();
        expiryMs    = par("registryExpiryMs");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        auto *listener = new TcpSocket();
        listener->setOutputGate(gate("socketOut"));
        listener->setCallback(this);
        listener->bind(localPort);
        listener->listen();
        socketMap[listener->getSocketId()] = listener;

        L3AddressResolver().tryResolve(par("registryAddress"), registryAddr);
        registryPort = par("registryPort");
    }
}

void GeoLoadBalancerApp::handleMessageWhenUp(cMessage *msg)
{
    TcpSocket *sock = nullptr;
    for (auto &p : socketMap)
        if (p.second->belongsToSocket(msg)) { sock = p.second; break; }

    if (sock) sock->processMessage(msg);
    else delete msg;
}

void GeoLoadBalancerApp::socketEstablished(TcpSocket *socket)
{
    socketMap[socket->getSocketId()] = socket;
}

void GeoLoadBalancerApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool)
{
    auto chunk = pkt->peekAtFront<BytesChunk>();
    const auto &bytes = chunk->getBytes();
    std::string req(bytes.begin(), bytes.end());
    delete pkt;

    // Parse path
    std::smatch m;
    std::string path = "/";
    if (std::regex_search(req, m, std::regex("GET\\s+([^\\s]+)\\s+HTTP/1")))
        path = m[1];

    // Parse X-Client-Geo
    double clat = 0, clon = 0;
    if (std::regex_search(req, m, std::regex("X-Client-Geo:\\s*([-0-9\\.]+),([-0-9\\.]+)"))) {
        clat = std::stod(m[1]);
        clon = std::stod(m[2]);
    }

    ensureRegistryCache();
    auto chosen = chooseMirror(clat, clon);
    reply302(socket, chosen, path);
}

void GeoLoadBalancerApp::socketClosed(TcpSocket *socket)
{
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void GeoLoadBalancerApp::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "LB socket failure code=" << code << "\n";
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void GeoLoadBalancerApp::ensureRegistryCache()
{
    if (simTime() - lastFetch < SimTime(expiryMs / 1000.0))
        return;

    // For now, static demo mirrors (registry integration can be added later)
    mirrors.clear();
    mirrors.push_back({"mirror0", 23.75, 90.39, 0.3});   // Dhaka, Bangladesh
    mirrors.push_back({"mirror1", 35.68, 139.69, 0.5});  // Tokyo, Japan
    mirrors.push_back({"mirror2", 51.51, -0.13, 0.4});   // London, UK
    lastFetch = simTime();
}

MirrorRow GeoLoadBalancerApp::chooseMirror(double clat, double clon)
{
    if (mirrors.empty())
        return MirrorRow{"fallback", 0, 0, 1.0};

    double maxLoad = 0.0001;
    for (auto &m : mirrors) maxLoad = std::max(maxLoad, m.load);

    double bestScore = 1e100;
    MirrorRow best = mirrors.front();

    for (auto &m : mirrors) {
        double dkm = haversine_km(clat, clon, m.lat, m.lon);
        double dnorm = dkm / 20000.0;
        double lnorm = m.load / maxLoad;
        double score = wDist * dnorm + wLoad * lnorm;
        if (score < bestScore) { bestScore = score; best = m; }
    }
    return best;
}

void GeoLoadBalancerApp::reply302(TcpSocket *socket, const MirrorRow &m, const std::string &path)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 302 Found\r\n"
        << "Location: http://" << m.id << ":8081" << path << "\r\n"
        << "X-Chosen-Mirror: " << m.id << "\r\n"
        << "Content-Length: 0\r\n\r\n";

    std::string s = oss.str();
    std::vector<uint8_t> data(s.begin(), s.end());
    auto payload = makeShared<BytesChunk>(data);

    auto pkt = new Packet("http-302");
    pkt->insertAtBack(payload);
    socket->send(pkt);
}

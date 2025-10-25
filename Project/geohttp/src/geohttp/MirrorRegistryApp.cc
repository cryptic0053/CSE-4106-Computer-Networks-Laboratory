#include "MirrorRegistryApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include <regex>
#include <sstream>

using namespace inet;

Define_Module(MirrorRegistryApp);

void MirrorRegistryApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        if (hasPar("expiryMs")) expiryMs = par("expiryMs");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        auto *listener = new TcpSocket();
        listener->setOutputGate(gate("socketOut"));
        listener->setCallback(this);
        listener->bind(localPort);
        listener->listen();
        socketMap[listener->getSocketId()] = listener;
    }
}

void MirrorRegistryApp::handleMessageWhenUp(cMessage *msg)
{
    TcpSocket *sock = nullptr;
    for (auto &p : socketMap)
        if (p.second->belongsToSocket(msg)) { sock = p.second; break; }

    if (sock) sock->processMessage(msg);
    else delete msg;
}

void MirrorRegistryApp::socketEstablished(TcpSocket *socket)
{
    socketMap[socket->getSocketId()] = socket;
}

void MirrorRegistryApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool)
{
    auto chunk = pkt->peekAtFront<BytesChunk>();
    const auto &bytes = chunk->getBytes();
    std::string s(bytes.begin(), bytes.end());
    delete pkt;

    if (s.rfind("REPORT", 0) == 0)
        handleReport(s);
    else if (s.rfind("GET /mirrors", 0) == 0)
        handleList(socket);
}

void MirrorRegistryApp::socketClosed(TcpSocket *socket)
{
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void MirrorRegistryApp::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "Registry socket failure id=" << socket->getSocketId()
            << " code=" << code << "\n";
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void MirrorRegistryApp::handleReport(const std::string& line)
{
    std::regex kvre("(id|lat|lon|load|ok)=([^\\s]+)");
    MirrorInfo info;

    info.lastSeen = simTime();
    for (auto i = std::sregex_iterator(line.begin(), line.end(), kvre);
         i != std::sregex_iterator(); ++i)
    {
        auto k = (*i)[1].str();
        auto v = (*i)[2].str();
        if (k == "id") info.id = v;
        else if (k == "lat") info.lat = std::stod(v);
        else if (k == "lon") info.lon = std::stod(v);
        else if (k == "load") info.load = std::stod(v);
        else if (k == "ok") info.ok = (v == "1");
    }
    if (!info.id.empty())
        db[info.id] = info;
}

void MirrorRegistryApp::handleList(TcpSocket *socket)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n[";

    bool first = true;
    for (auto &kv : db) {
        auto &mi = kv.second;
        if (!mi.ok) continue;
        if ((simTime() - mi.lastSeen).inUnit(SIMTIME_MS) > expiryMs) continue;
        if (!first) oss << ",";
        first = false;
        oss.setf(std::ios::fixed); oss.precision(6);
        oss << "{\"id\":\"" << mi.id << "\",\"lat\":" << mi.lat
            << ",\"lon\":" << mi.lon << ",\"load\":" << mi.load << "}";
    }
    oss << "]";

    std::string resp = oss.str();
    std::vector<uint8_t> data(resp.begin(), resp.end());
    auto payload = makeShared<BytesChunk>(data);

    auto pktOut = new Packet("mirrors");
    pktOut->insertAtBack(payload);
    socket->send(pktOut);
}

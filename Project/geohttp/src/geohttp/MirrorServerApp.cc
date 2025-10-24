#include "MirrorServerApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include <sstream>
#include <vector>

using namespace inet;

Define_Module(MirrorServerApp);

void MirrorServerApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        if (hasPar("localPort")) localPort = par("localPort");
        if (hasPar("mirrorId"))  mirrorId  = par("mirrorId").stdstringValue();
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

void MirrorServerApp::handleMessageWhenUp(cMessage *msg)
{
    TcpSocket *sock = nullptr;
    for (auto &p : socketMap)
        if (p.second->belongsToSocket(msg)) { sock = p.second; break; }

    if (sock) sock->processMessage(msg);
    else delete msg;
}

void MirrorServerApp::socketEstablished(TcpSocket *socket)
{
    socketMap[socket->getSocketId()] = socket;
}

void MirrorServerApp::socketDataArrived(TcpSocket *socket, Packet *pkt, bool)
{
    auto chunk = pkt->peekAtFront<BytesChunk>();
    const auto &bytes = chunk->getBytes();
    std::string req(bytes.begin(), bytes.end());
    delete pkt;

    std::ostringstream body;
    body << "served-by=" << (mirrorId.empty() ? std::to_string(socket->getSocketId()) : mirrorId) << "\n";

    reply200(socket, body.str());
    socket->close();
}

void MirrorServerApp::socketClosed(TcpSocket *socket)
{
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void MirrorServerApp::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "MirrorServer socket failure id=" << socket->getSocketId()
            << " code=" << code << "\n";
    socketMap.erase(socket->getSocketId());
    delete socket;
}

void MirrorServerApp::reply200(TcpSocket *socket, const std::string& body)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;

    std::string s = oss.str();
    std::vector<uint8_t> data(s.begin(), s.end());
    auto payload = makeShared<BytesChunk>(data);

    auto pkt = new Packet("http-200");
    pkt->insertAtBack(payload);
    socket->send(pkt);
}

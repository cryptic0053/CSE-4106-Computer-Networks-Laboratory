#include "MirrorReporterApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include <vector>
#include <sstream>

using namespace inet;

Define_Module(MirrorReporterApp);

void MirrorReporterApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registryPort = par("registryPort");
        mirrorId = par("mirrorId").stdstringValue();
        if (hasPar("latitude"))  latitude  = par("latitude");
        if (hasPar("longitude")) longitude = par("longitude");
        if (hasPar("load"))      load      = par("load");
        if (hasPar("ok"))        ok        = par("ok");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        L3AddressResolver().tryResolve(par("registryAddress"), registryAddr);

        auto *socket = new TcpSocket();
        socket->setOutputGate(gate("socketOut"));
        socket->setCallback(this);
        socket->connect(registryAddr, registryPort);
    }
}

void MirrorReporterApp::handleMessageWhenUp(cMessage *msg)
{
    if (auto ind = dynamic_cast<TcpAvailableInfo *>(msg->getControlInfo())) {
        auto *socket = new TcpSocket(ind);
        socket->setOutputGate(gate("socketOut"));
        socket->setCallback(this);
        socket->processMessage(msg);
    } else {
        delete msg;
    }
}

void MirrorReporterApp::socketEstablished(TcpSocket *socket)
{
    std::ostringstream oss;
    oss << "REPORT id=" << mirrorId
        << " lat=" << latitude
        << " lon=" << longitude
        << " load=" << load
        << " ok=" << (ok ? 1 : 0) << "\r\n";

    std::string msg = oss.str();
    std::vector<uint8_t> bytes(msg.begin(), msg.end());
    auto payload = makeShared<BytesChunk>(bytes);

    auto pkt = new Packet("mirror-report");
    pkt->insertAtBack(payload);
    socket->send(pkt);

    socket->close();
}

void MirrorReporterApp::socketDataArrived(TcpSocket *, Packet *pkt, bool)
{
    delete pkt;  // reporter doesn't expect responses
}

void MirrorReporterApp::socketClosed(TcpSocket *socket)
{
    delete socket;
}

void MirrorReporterApp::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "MirrorReporter socket failure code=" << code << "\n";
    delete socket;
}

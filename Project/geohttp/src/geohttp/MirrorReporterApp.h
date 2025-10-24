#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

using namespace inet;

class MirrorReporterApp : public TcpAppBase, public TcpSocket::ICallback
{
  protected:
    L3Address registryAddr;
    int registryPort;
    std::string mirrorId;
    double geoLat, geoLon;
    simtime_t reportInterval;

    TcpSocket socket;
    bool connected = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void connectIfNeeded();
    void sendReport();

    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
};

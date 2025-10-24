#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpSocketMap.h"

using namespace inet;

class MirrorServerApp : public TcpAppBase, public TcpSocket::ICallback
{
  protected:
    int localPort;
    std::string mirrorId;
    double geoLat, geoLon;
    double serviceRateBps;

    TcpSocketMap socketMap;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // TcpSocket::ICallback
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    void replyOk(TcpSocket *socket, int objectBytes);
};

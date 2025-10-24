#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

using namespace inet;

class GeoClientApp : public TcpAppBase, public TcpSocket::ICallback
{
  protected:
    L3Address connectAddress;
    int connectPort;
    double geoLat, geoLon;
    simtime_t nextReqInterval;
    int objectSize;

    TcpSocket socket;
    bool connected = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void sendRequest();

    // TcpSocket::ICallback
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
};

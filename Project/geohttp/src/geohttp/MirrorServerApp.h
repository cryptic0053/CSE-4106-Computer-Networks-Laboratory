#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETDefs.h"
#include <map>
#include <string>

using namespace inet;

class MirrorServerApp : public TcpAppBase
{
  protected:
    int localPort = 8081;
    std::string mirrorId;
    std::map<int, TcpSocket*> socketMap;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *op) override {}
    virtual void handleStopOperation(LifecycleOperation *op) override {}
    virtual void handleCrashOperation(LifecycleOperation *op) override {}
    virtual void handleTimer(cMessage *msg) override {}

    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    void reply200(TcpSocket *socket, const std::string& body);
};

#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/INETDefs.h"
#include <string>

using namespace inet;

class MirrorReporterApp : public TcpAppBase
{
  protected:
    // Params
    L3Address registryAddr;
    int registryPort = -1;
    std::string mirrorId;
    double latitude = 0;
    double longitude = 0;
    double load = 0;
    bool ok = true;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // Callbacks
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    // Lifecycle stubs
    virtual void handleTimer(cMessage *msg) override {}
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
};

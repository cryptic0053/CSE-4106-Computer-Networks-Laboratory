#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/INETDefs.h"
#include <string>
#include <vector>

using namespace inet;

class GeoClientApp : public TcpAppBase
{
  protected:
    // Params
    std::string connectAddressStr;
    L3Address connectAddress;
    int connectPort = -1;
    double latitude = 0;
    double longitude = 0;
    std::string path = "/";

    // State
    TcpSocket *socket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // Lifecycle stubs (required by TcpAppBase / OperationalMixin)
    virtual void handleStartOperation(LifecycleOperation *op) override {}
    virtual void handleStopOperation(LifecycleOperation *op) override {}
    virtual void handleCrashOperation(LifecycleOperation *op) override {}
    virtual void handleTimer(cMessage *msg) override {}

    // Callbacks
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    void sendRequest();
};

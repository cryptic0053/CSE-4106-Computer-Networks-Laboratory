#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETDefs.h"
#include <map>
#include <string>

using namespace inet;

struct MirrorInfo {
    std::string id;
    double lat = 0;
    double lon = 0;
    double load = 0;
    bool ok = true;
    simtime_t lastSeen = SIMTIME_ZERO;
};

class MirrorRegistryApp : public TcpAppBase
{
  protected:
    int localPort = -1;
    int expiryMs = 5000;

    std::map<int, TcpSocket*> socketMap;
    std::map<std::string, MirrorInfo> db;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // Callbacks
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    // Helpers
    void handleReport(const std::string& line);
    void handleList(TcpSocket *socket);

    // Lifecycle stubs
    virtual void handleTimer(cMessage *msg) override {}
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
};

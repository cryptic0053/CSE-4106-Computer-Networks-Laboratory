#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <map>
#include <vector>
#include <string>

using namespace inet;

struct MirrorRow {
    std::string id;
    double lat = 0;
    double lon = 0;
    double load = 0;
};

class GeoLoadBalancerApp : public TcpAppBase
{
  protected:
    // Server settings
    int localPort = -1;
    std::string mode;   // "302" or "proxy"
    std::map<int, TcpSocket*> socketMap;

    // Registry info
    L3Address registryAddr;
    int registryPort = -1;
    double wDist = 1.0;
    double wLoad = 1.0;
    double wDown = 1.0;
    int expiryMs = 5000;

    // Mirror cache
    std::vector<MirrorRow> mirrors;
    simtime_t lastFetch = SIMTIME_ZERO;

  protected:
    // Lifecycle
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override {}
    virtual void handleCrashOperation(LifecycleOperation *operation) override {}
    virtual void handleTimer(cMessage *msg) override {}

    // Logic
    void ensureRegistryCache();
    MirrorRow chooseMirror(double clat, double clon);
    void reply302(TcpSocket *socket, const MirrorRow& m, const std::string& path);

    // Callbacks
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
};

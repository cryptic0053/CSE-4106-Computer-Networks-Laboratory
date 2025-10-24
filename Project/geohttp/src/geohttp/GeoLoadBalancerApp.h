#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpSocketMap.h"
#include "GeoMath.h"

#include <vector>

using namespace inet;

struct MirrorRow {
    std::string id;
    double lat=0, lon=0;
    double load=0;
};

class GeoLoadBalancerApp : public TcpAppBase, public TcpSocket::ICallback
{
  protected:
    // server side
    int localPort;
    std::string mode; // "302" or "proxy"
    TcpSocketMap socketMap;

    // registry
    L3Address registryAddr;
    int registryPort;
    double wDist, wLoad, wDown;
    int expiryMs;

    // cache
    std::vector<MirrorRow> mirrors;
    simtime_t lastFetch = SIMTIME_ZERO;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    void ensureRegistryCache();
    void fetchMirrors();

    MirrorRow chooseMirror(double clat, double clon);

    void reply302(TcpSocket *socket, const MirrorRow& m, const std::string& path);

    // TcpSocket::ICallback
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
};

#pragma once
#include "inet/applications/tcpapp/TcpAppBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpSocketMap.h"

#include <unordered_map>

using namespace inet;

struct MirrorInfo {
    std::string id;
    double lat = 0, lon = 0;
    double load = 0;
    bool ok = false;
    simtime_t lastSeen;
};

class MirrorRegistryApp : public TcpAppBase, public TcpSocket::ICallback
{
  protected:
    int localPort;
    int expiryMs;

    TcpSocketMap socketMap;
    std::unordered_map<std::string, MirrorInfo> db;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // TcpSocket::ICallback
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketDataArrived(TcpSocket *socket, Packet *pkt, bool urgent) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;

    void handleReport(const std::string& line);
    void handleList(TcpSocket *socket);
};

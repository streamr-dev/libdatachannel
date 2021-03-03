//
//  peer.h
//  libdatachannel
//
//  Created by Savolainen, Petri T E on 01/03/2021.
//

#ifndef peer_h
#define peer_h

#include <string>
#include <nlohmann/json.hpp>
#include "rtc/rtc.hpp"

#define BUFFER_LOW 1<<20
#define BUFFER_HIGH 1<<24
#define PACKAGE_SIZE 800
#define INTERVAL 2

using namespace std;
using namespace rtc;
using json = nlohmann::json;

class Peer
{
private:
    string selfId;
    string peerId;
    string logId;
    
    shared_ptr<PeerConnection> connection;
    shared_ptr<DataChannel> dc;
    shared_ptr<WebSocket> signalingServerWs;
    bool paused = true;
    
    PeerConnection::State lastState;
    int bytesIn = 0;
    int bytesOut = 0;
    int bytesFailed = 0;
    
    shared_ptr<PeerConnection> createPeerConnection(const Configuration &config, weak_ptr<WebSocket> wws, string id);
    void setUpDataChannel(shared_ptr<DataChannel> dc);
    void sendRelay(json& data);
    
public:
    Peer(string selfId, string peerId, shared_ptr<WebSocket> signalingServerWs);

    void startAsActive();
    void startAsPassive();
    
    void handleRemoteData(json data);
    bool publish(const string& message);
    
    string getLogId();
    int getBufferedAmount();
    PeerConnection::State getState();
    bool isOpen();
    int getBytesIn();
    int getBytesOut();
    int getBytesFailed();
    void resetCounters();
};


#endif /* peer_h */

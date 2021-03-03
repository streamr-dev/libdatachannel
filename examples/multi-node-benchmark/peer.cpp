//
//  peer.cpp
//  datachannel
//
//  Created by Savolainen, Petri T E on 01/03/2021.
//

#include <stdio.h>
#include "peer.h"

Peer::Peer(string _selfId, string _peerId, shared_ptr<WebSocket> _signalingServerWs):
    selfId(_selfId), peerId(_peerId), signalingServerWs(_signalingServerWs)
	{
	this->logId = this->selfId + "->" + this->peerId;
       
    string stunServer = "stun:stun.l.google.com:19302";
    Configuration config;
    config.iceServers.emplace_back(stunServer);
    this->connection = make_shared<PeerConnection>(config);
        
    this->connection->onStateChange([this](PeerConnection::State state)
        {
        cout << "State: " << state << endl;
        this->lastState = state;
        });
    
   
    this->connection->onLocalDescription([_signalingServerWs, _selfId, _peerId](Description description)
        {
        json data =  {{"id", _selfId}, {"type", description.typeString()}, {"description", string(description)}};
        json message =
            {
                {"type", "relay"},
                {"to", _peerId},
                {"data", data}
            };
        
        _signalingServerWs->send(message.dump());
        });
       
    this->connection->onLocalCandidate([_signalingServerWs, _selfId, _peerId](Candidate candidate)
        {
        json data = { {"candidate", candidate}, {"mid",candidate.mid()} };
        
        json message =
                   {
                       {"type", "relay"},
                       {"to", _peerId},
                       {"data", data}
                   };
               
        _signalingServerWs->send(message.dump());
        });
   }

void Peer::setUpDataChannel(shared_ptr<DataChannel> dc)
    {
    dc->setBufferedAmountLowThreshold(BUFFER_LOW);
        
    dc->onOpen([logId = this->logId, peerId = this->peerId]()
        {
        cout << logId << " DataChannel " << peerId << " open" << endl;
        });
        
    dc->onClosed([logId = this->logId, peerId = this->peerId]()
        {
        cout << logId << " DataChannel " << peerId << " closed" << endl;
        });
        
    dc->onError([logId = this->logId, peerId = this->peerId](string err)
        {
        cout << logId << " DataChannel " << peerId << " error: "<< err << endl;
        });
        
    dc->onBufferedAmountLow([obj = this, logId = this->logId, peerId = this->peerId, dc= this->dc] ()
        {
        obj->paused = false;
        cout << logId << " DataChannel " << peerId << " LOW buffer " <<  dc->bufferedAmount()<< endl;
        });
        
        
    dc->onMessage([obj=this] (message_variant msg)
        {
        if (holds_alternative<string>(msg))
            {
            obj->bytesIn += get<string>(msg).size();
            }
        });
        
    this->dc = dc;
    }

void Peer::startAsActive()
	{
	if (this->dc)
		{
		throw runtime_error("Already started!");
		}
    
    this->dc =  this->connection->createDataChannel("generalDataChannel");
        
    this->setUpDataChannel(this->dc);
    
    dc->onOpen([obj=this]()
        {
        obj->paused = false;
        });
    }

void Peer::startAsPassive()
    {
    if (this->dc)
        {
        throw runtime_error("Already started!");
        }
        
    this->connection->onDataChannel([this](shared_ptr<DataChannel> dc)
        {
        cout << "DataChannel received with label \"" << dc->label() << "\"" << endl;

        this->setUpDataChannel(dc);
        this->paused = false;
        });
    }

void Peer::handleRemoteData(json data)
    {
    string candidate = "";
    string mid = "";
    string description = "";
    string type = "";
        
    auto it = data.find("candidate");
    if (it != data.end())
        {
        candidate = it->get<string>();
        }
   
    auto it2 = data.find("mid");
    if (it2 != data.end())
        {
        mid = it2->get<string>();
        }
   
    auto it3 = data.find("description");
    if (it3 != data.end())
        {
        description = it3->get<string>();
        }
           
    auto it4 = data.find("type");
    if (it4 != data.end())
        {
        type = it4->get<string>();
        }
    
    if (candidate != "" && mid !="")
        {
        this->connection->addRemoteCandidate(candidate);
        }
    
   else if (description != "" && type!= "")
        {
        Description desc(description, type);
        this->connection->setRemoteDescription(desc);
        }
   else
        {
        cout << this->logId << " unrecognized RTC message: " << data << endl;
        }
    }

bool Peer::publish(const string& message)
    {
    if (this->paused)
        {
        this->bytesFailed += message.size();
        return false;
        }
        
    if (this->dc->bufferedAmount() >= BUFFER_HIGH)
        {
        this->paused = true;
        cout << this->logId << " DataChannel HIGH buffer " << this->dc->bufferedAmount() <<endl;
        this->bytesFailed += message.size();
        return false;
       }
        
	try {
		this->dc->send(message);
	} catch (const std::exception& e) {
		cout << this->logId << " send failed: " << e.what() << endl;
		this->bytesFailed += message.size();
	}
    this->bytesOut += message.size();
    return true;
    }


string Peer::getLogId()
    {
    return this->logId;
    }

int Peer::getBufferedAmount()
    {
    return this->dc ? this->dc->bufferedAmount() : 0;
    }

PeerConnection::State Peer::getState()
    {
    return this->lastState;
    }

bool Peer::isOpen()
    {
    return this->dc ? this->dc->isOpen() : false;
    }

int Peer::getBytesIn()
    {
    return this->bytesIn;
    }

int Peer::getBytesOut()
    {
    return this->bytesOut;
    }

int Peer::getBytesFailed()
    {
    return this->bytesFailed;
    }

void Peer::resetCounters()
    {
    this->bytesIn = 0;
    this->bytesOut = 0;
    this->bytesFailed = 0;
    }





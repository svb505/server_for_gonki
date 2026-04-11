#include <enet/enet.h>
#include <iostream>
#include <unordered_map>
#include "common.h"
#include <algorithm>
#include <vector>
#include <deque>
#include "myMath.h"
#include <cstring>

const size_t HISTORY_SIZE = 10;

struct Player{
    ENetPeer* peer;
    CarState state;
    std::deque<CarState> history; 
};

std::unordered_map<ENetPeer*, Player> players;
uint32_t nextId = 1;
std::vector<std::string> messages;

ChatPacket lastChat{};
bool hasChat = false;

void SendSnapshot(ENetHost* server) {
    SnapshotPacket snap{};
    snap.type = PacketType::Snapshot;
    snap.count = 0;

    for (auto& [peer, player] : players) {
        if (snap.count >= MAX_PLAYERS) break;

        CarState correctState = player.state;
        if (!player.history.empty())
            correctState = player.history.back();

        snap.cars[snap.count++] = correctState;
    }

    ENetPacket* packet = enet_packet_create(&snap, sizeof(SnapshotPacket), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(server, 0, packet);

    if (hasChat) {
        ENetPacket* chatPacket = enet_packet_create(&lastChat, sizeof(ChatPacket), ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(server, 1, chatPacket);

        hasChat = false;
    }
}


int main(){
    enet_initialize();
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 7777;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

    if (!server){
        std::cout << "Server create failed\n";
        return 1;
    }

    std::cout << "Server started\n";

    ENetEvent event;

    while (true){
        while (enet_host_service(server, &event, 1) > 0){
            switch (event.type){
            case ENET_EVENT_TYPE_CONNECT:{
                Player p{};
                p.peer = event.peer;
                p.state.id = nextId++;
                p.state.x = 0;
                p.state.y = 0;
                p.state.z = 0;  
                p.state.angle = 0;
                p.state.speed = 0;
                p.state.lastCheckpoint = checkpoints.size() - 1;
                p.state.lap = 0;

                players[event.peer] = p;

                ClientStatePacket init{};
                init.type = PacketType::ClientState;
                init.state = p.state;

                ENetPacket* packet = enet_packet_create(&init,sizeof(init),ENET_PACKET_FLAG_UNSEQUENCED);
                enet_peer_send(event.peer, 0, packet);

                std::cout << "Client connected\n";
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                PacketType type = *(PacketType*)event.packet->data;

                if (type == PacketType::Chat) {
                    auto* p = (ChatPacket*)event.packet->data;
                    messages.push_back(p->msg);

                    lastChat = *p;
                    hasChat = true;
                }
                if (type == PacketType::ClientState) {
                    auto* packet = (ClientStatePacket*)event.packet->data;
                    Player& p = players[event.peer];

                    p.state.x = packet->state.x;
                    p.state.y = packet->state.y;
                    p.state.z = packet->state.z;
                    p.state.angle = packet->state.angle;
                    p.state.speed = packet->state.speed;

                    p.history.push_back(p.state);
                    if (p.history.size() > HISTORY_SIZE) p.history.pop_front();

                    updateProgress(players[event.peer].state, checkpoints);
                }

                enet_packet_destroy(event.packet);
                break;
            }


            case ENET_EVENT_TYPE_DISCONNECT:
                players.erase(event.peer);
                std::cout << "Client disconnected\n";
                break;
            }
        }

        SendSnapshot(server);
    }

    enet_host_destroy(server);
}

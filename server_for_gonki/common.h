#pragma once
#include <cstdint>

enum class PacketType : uint8_t
{
    ClientState = 1,
    Snapshot = 2
};

struct CarState
{
    uint32_t id;
    float x;
    float y;
    float z;
    float speed;
    float angle;
};

struct ClientStatePacket
{
    PacketType type;
    CarState state;
};

const int MAX_PLAYERS = 8;

struct SnapshotPacket
{
    PacketType type;
    uint32_t count;
    CarState cars[MAX_PLAYERS];
};

#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

struct Vec2 {
    float x, y;
};
struct RoadStyle {
    float width;
};
struct Checkpoint {
    Vec2 pos;
    float radius;
};
std::vector<Checkpoint> checkpoints = {
    { {35, 0}, 10.0f },
    { {95, 0}, 10.0f },
    { {95, 35}, 10.0f },
    { {130, 35}, 10.0f },
    { {130, 95}, 10.0f },
    { {95, 95}, 10.0f },
    { {35, 130}, 10.0f },
    { {0, 95}, 10.0f },
    { {0, 35}, 10.0f }
};

float distance(const CarState& car, const Vec2& cp) {
    float dx = car.x - cp.x;
    float dy = car.z - cp.y;
    return std::sqrt(dx * dx + dy * dy);
}
void updateProgress(CarState& car, const std::vector<Checkpoint>& checkpoints) {
    int totalCheckpoints = checkpoints.size();

    int nextCp = (car.lastCheckpoint + 1) % totalCheckpoints;
    const Checkpoint& cp = checkpoints[nextCp];

    float dx = car.x - cp.pos.x;
    float dz = car.z - cp.pos.y;
    float dist = std::sqrt(dx * dx + dz * dz);

    if (dist < cp.radius * 1.5f) {
        car.lastCheckpoint = nextCp;
        if (nextCp == 0) car.lap++;
    }

    const Vec2& cur = checkpoints[car.lastCheckpoint].pos;
    const Vec2& next = checkpoints[nextCp].pos;

    float Nextdx = next.x - cur.x;
    float Nextdz = next.y - cur.y;
    float segmentLen = std::sqrt(Nextdx * Nextdx + Nextdz * Nextdz);
    float toNext = std::sqrt((next.x - car.x) * (next.x - car.x) + (next.y - car.z) * (next.y - car.z));

    car.progress = 1.0f - std::clamp(toNext / (segmentLen + 0.0001f), 0.0f, 1.0f);
}

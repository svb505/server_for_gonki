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
    { {35, 0}, 2.0f },
    { {95, 0}, 2.0f },
    { {95, 35}, 2.0f },
    { {130, 35}, 2.0f },
    { {130, 95}, 2.0f },
    { {95, 95}, 2.0f },
    { {35, 130}, 2.0f },
    { {0, 95}, 2.0f },
    { {0, 35}, 2.0f }
};

float distance(const CarState& car, const Vec2& cp) {
    float dx = car.x - cp.x;
    float dy = car.z - cp.y;  // если z — это "y" на дороге
    return std::sqrt(dx * dx + dy * dy);
}
void updateProgress(CarState& car, const std::vector<Checkpoint>& checkpoints) {
    int nextCp = (car.lastCheckpoint + 1) % checkpoints.size();

    float d = distance(car, checkpoints[nextCp].pos);
    float radius = checkpoints[nextCp].radius;

    if (d < radius) {
        car.lastCheckpoint = nextCp;
        if (nextCp == 0) car.lap++;
    }

    const Vec2& cur = checkpoints[car.lastCheckpoint].pos;
    const Vec2& next = checkpoints[nextCp].pos;
    float totalDist = distance({ 0,0,0,0,0,0,0,0 }, next) + 0.0001f;
    float passedDist = distance({ 0,0,0,0,0,0,0,0 }, cur) + 0.0001f;

    float dx = next.x - cur.x;
    float dy = next.y - cur.y;
    float segmentLen = std::sqrt(dx * dx + dy * dy);
    float toNext = std::sqrt((next.x - car.x) * (next.x - car.x) + (next.y - car.z) * (next.y - car.z));
    car.progress = 1.0f - std::clamp(toNext / (segmentLen + 0.0001f), 0.0f, 1.0f);
}
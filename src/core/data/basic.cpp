#include "basic.h"

namespace Data {
    bool Vec2::operator==(const Vec2& other) {
        return x == other.x && y == other.y;
    }

    bool Vec3::operator==(const Vec3& other) {
        return x == other.x && y == other.y && z == other.z;
    }
}
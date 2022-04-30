#pragma once

#include <vector>

namespace Data {
    struct Vec2 {
        float x;
        float y;

        bool operator==(const Vec2& other);
    };


    struct Vec3 {
        float x;
        float y;
        float z;

        bool operator==(const Vec3& other);
    };

    enum OrientationNames {
        AXIAL, SAGITAL, CORONAL
    };

    const Vec3 Orientations[3] = {
        Vec3(0.f, 0.f, 1.f),
        Vec3(0.f, 1.f, 0.f),
        Vec3(1.f, 0.f, 0.f)
    };
}

namespace std {

    template <>
    struct hash<Data::Vec2> {
        std::size_t operator()(const Data::Vec2& k) const {
            using std::size_t;
            using std::hash;

            return ((hash<float>()(k.x)
                ^ (hash<float>()(k.y) << 1)) >> 1);
        }
    };

    template <>
    struct hash<Data::Vec3> {
        std::size_t operator()(const Data::Vec3& k) const {
            using std::size_t;
            using std::hash;

            return ((hash<float>()(k.x)
                ^ (hash<float>()(k.y) << 1)) >> 1)
                ^ (hash<float>()(k.z) << 1);
        }
    };

}
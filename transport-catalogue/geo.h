#pragma once
#include <cmath>
#include <iostream>
#include <string>

const int EARTH_RADIUS = 6371000;
const double PI = 3.1415926535;
namespace transport {
    namespace geo {
        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const {
                return lat == other.lat && lng == other.lng;
            }
            bool operator!=(const Coordinates& other) const {
                return !(*this == other);
            }
        };

        struct Hasher {
            size_t operator()(Coordinates key) const {
                return static_cast<size_t>(key.lat + key.lng);
            }
        };

        inline double ComputeDistance(Coordinates from, Coordinates to) {
            using namespace std;
            if (from == to) {
                return 0;
            }
            static const double dr = PI / 180.;
            return acos(sin(from.lat * dr) * sin(to.lat * dr) + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * EARTH_RADIUS;
        }
    }
}

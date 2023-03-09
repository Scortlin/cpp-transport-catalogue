#pragma once

#include <cstdlib>

namespace geo{
const int EARTH_RADIUS = 6371000;

struct Coordinates{
    double lat;
    double lng;  
    bool operator==(const Coordinates&) const;
    bool operator!=(const Coordinates&) const;
};

class CoordinatesHasher{
public:
    std::size_t operator()(const Coordinates&) const;
};

double ComputeDistance(Coordinates from, Coordinates to);
}  
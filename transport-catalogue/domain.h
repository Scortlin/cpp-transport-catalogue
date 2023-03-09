#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>   

namespace transport_catalogue{
struct Stop;      
struct Route;     

struct Stop{
	Stop() = default;
	Stop(const std::string_view stop_name, const double lat, const double lng);
	Stop(const Stop* other_stop_ptr);
	std::string name;
	geo::Coordinates coords{0,0};
};


struct Route{
	Route() = default;
	Route(const Route* other_stop_ptr);

	std::string route_name;
	std::vector<const Stop*> stops;
	size_t unique_stops_qty = 0;
	double geo_route_length = 0;
	size_t meters_route_length = 0;
	double curvature = 0;
	bool is_circular = false;
};

struct RendererData{
	std::vector<geo::Coordinates> stop_coords;
	std::vector<std::string_view> stop_names;  
	bool is_circular = false; 
};
class Hasher{
public:
	std::size_t operator()(const std::pair<const Stop*, const Stop*> pair_of_pointers) const noexcept;
private:
	std::hash<const void*> hasher_;
};
}
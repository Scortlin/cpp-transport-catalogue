#include "domain.h"
using namespace std;
namespace transport_catalogue{

Stop::Stop(const Stop* other_stop_ptr) :
	name(other_stop_ptr->name),
	coords(other_stop_ptr->coords)
{}

Stop::Stop(const string_view stop_name, const double lat, const double lng) : 
	name(stop_name), 
	coords(geo::Coordinates{ lat, lng })
{}


Route::Route(const Route* other_stop_ptr) :
    route_name(other_stop_ptr->route_name),
	stops(other_stop_ptr->stops),
	unique_stops_qty(other_stop_ptr->unique_stops_qty),
	geo_route_length(other_stop_ptr->geo_route_length),
	meters_route_length(other_stop_ptr->meters_route_length),
	curvature(other_stop_ptr->curvature),
	is_circular(other_stop_ptr->is_circular)
{}

size_t Hasher::operator()(const pair<const Stop*, const Stop*> pair_of_pointers) const noexcept{
	return hasher_(static_cast<const void*>(pair_of_pointers.first)) * 37
		+ hasher_(static_cast<const void*>(pair_of_pointers.second));
}

} 
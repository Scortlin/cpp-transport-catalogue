#include "transport_catalogue.h"
#include <algorithm>   
using namespace std;
namespace transport_catalogue{
	StopStat::StopStat(string_view stop_name, set<string_view>& buses) :
		name(stop_name), buses(buses)
	{}

	RouteStat::RouteStat(size_t stops, size_t unique_stops, int64_t meters_length, double curvature, string_view name) :
		stops_on_route(stops),
		unique_stops(unique_stops),
		meters_route_length(meters_length),
		curvature(curvature),
		name(name)
	{}

	TransportCatalogue::TransportCatalogue(){}
	TransportCatalogue::~TransportCatalogue(){}

	void TransportCatalogue::AddStop(Stop&& stop){
		if (all_stops_map_.count(GetStopName(&stop)) == 0){
			auto& ref = all_stops_data_.emplace_back(move(stop));
			all_stops_map_.insert({string_view(ref.name), &ref });
		}
	}

	void TransportCatalogue::AddRoute(Route&& route){
		if (all_buses_map_.count(route.route_name) == 0){
			auto& ref = all_buses_data_.emplace_back(move(route));
			all_buses_map_.insert({string_view(ref.route_name), &ref });
			vector<const Stop*> tmp = ref.stops;
			sort(tmp.begin(), tmp.end());
			auto last = unique(tmp.begin(), tmp.end());
			ref.unique_stops_qty = (last != tmp.end() ? std::distance(tmp.begin(), last) : tmp.size());
			if (!ref.is_circular){
				for (int i = ref.stops.size() - 2; i >= 0; --i){
					ref.stops.push_back(ref.stops[i]);
				}
			}
			int stops_num = static_cast<int>(ref.stops.size());
			if (stops_num > 1){
				ref.geo_route_length = 0;
				ref.meters_route_length = 0;
				for (int i = 0; i < stops_num - 1; ++i){
					ref.geo_route_length += ComputeDistance(ref.stops[i]->coords, ref.stops[i + 1]->coords);
					ref.meters_route_length += GetDistance(ref.stops[i], ref.stops[i + 1]);
				}
				ref.curvature = ref.meters_route_length / ref.geo_route_length;
			}
			else{
				ref.geo_route_length = 0;
				ref.meters_route_length = 0;
				ref.curvature = 1;
			}
		}
	}

	void TransportCatalogue::AddDistance(const Stop*stop_from, const Stop*stop_to, size_t dist){
		if (stop_from != nullptr && stop_to != nullptr){
			distances_map_.insert({ { stop_from, stop_to }, dist });
		}
	}

	size_t TransportCatalogue::GetDistance(const Stop*stop_from, const Stop*stop_to){
		size_t result = GetDistanceDirectly(stop_from, stop_to);
		return (result > 0 ? result : GetDistanceDirectly(stop_to, stop_from));
	}

	size_t TransportCatalogue::GetDistanceDirectly(const Stop*stop_from, const Stop*stop_to){
		if (distances_map_.count({ stop_from, stop_to }) > 0){
			return distances_map_.at({ stop_from, stop_to });
		}
		else{
			return 0;
		}
	}

	string_view TransportCatalogue::GetStopName(const Stop* stop_ptr){
		return string_view(stop_ptr->name);
	}

	string_view TransportCatalogue::GetStopName(const Stop stop){
		return string_view(stop.name);
	}

	string_view TransportCatalogue::GetBusName(const Route* route_ptr){
		return string_view(route_ptr->route_name);
	}

	string_view TransportCatalogue::GetBusName(const Route route){
		return string_view(route.route_name);
	}

	const Stop*TransportCatalogue::GetStopByName(const string_view stop_name) const{
		if (all_stops_map_.count(stop_name) == 0){
			return nullptr;
		}
		else{
			return all_stops_map_.at(stop_name);
		}
	}

	RouteStatPtr TransportCatalogue::GetRouteInfo(const string_view route_name) const{
		const Route* ptr = GetRouteByName(route_name);
		if (ptr == nullptr){
			return nullptr;
		}
		return new RouteStat(ptr->stops.size(),
			ptr->unique_stops_qty,
			ptr->meters_route_length,
			ptr->curvature,
			ptr->route_name);
	}
    
    const Route* TransportCatalogue::GetRouteByName(const string_view bus_name) const{
		if (all_buses_map_.count(bus_name) == 0){
			return nullptr;
		}
		else{
			return all_buses_map_.at(bus_name);
		}
	}
    
	StopStatPtr TransportCatalogue::GetBusesForStopInfo(const string_view stop_name) const{
		const Stop*ptr = GetStopByName(stop_name);
		if (ptr == nullptr){
			return nullptr;
		}
		set<string_view> found_buses;
		for (const auto& bus : all_buses_map_){
			auto tmp = find_if(bus.second->stops.begin(), bus.second->stops.end(),
				[stop_name](const Stop* curr_stop)
				{
					return (curr_stop->name == stop_name);
				});
			if (tmp != bus.second->stops.end()){
				found_buses.insert(bus.second->route_name);
			}
		}
		return new StopStat(stop_name, found_buses);
	}

	void TransportCatalogue::GetAllRoutes(map<const string, RendererData>& all_routes) const{
		for (const auto& route : all_buses_data_){
			if (route.stops.size() > 0){
				RendererData item;
				for (const Stop*stop : route.stops){
					item.stop_coords.push_back(stop->coords);
					item.stop_names.push_back(stop->name);
				}
				item.is_circular = route.is_circular;
				all_routes.emplace(make_pair(route.route_name, item));
			}
		}
		return;
	}

	size_t TransportCatalogue::GetAllStopsCount() const{
		return all_stops_data_.size();
	}

	const vector<const Stop*> TransportCatalogue::GetAllStopsPtr() const{
		vector<const Stop*> stop_ptrs;
		for (const auto& [stop_name, stop_ptr] : all_stops_map_){
			stop_ptrs.push_back(stop_ptr);
		}
		return stop_ptrs;
	}

	const deque<const Route*> TransportCatalogue::GetAllRoutesPtr() const{
		deque<const Route*> route_ptrs;
		for (const auto& route : all_buses_data_){
			route_ptrs.emplace_back(&route);
		}
		sort(route_ptrs.begin(), route_ptrs.end(), [](const Route* lhs, const Route* rhs)
			{
				return lhs->route_name <= rhs->route_name;
			});
		return route_ptrs;
	}

	const unordered_map<pair<const Stop*, const Stop*>, size_t, Hasher>& TransportCatalogue::GetAllDistances() const{
		return distances_map_;
	}

}
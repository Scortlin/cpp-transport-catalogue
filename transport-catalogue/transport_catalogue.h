#pragma once
#include "geo.h"          
#include "domain.h"       

#include <deque>
#include <map>             
#include <vector>
#include <string>
#include <string_view>
#include <set>
#include <algorithm>       
#include <utility>               
#include <ostream>         
#include <sstream>         
#include <iomanip>         
#include <unordered_set>
#include <unordered_map>

namespace transport_catalogue{
	struct StopStat{
		explicit StopStat(std::string_view, std::set<std::string_view>&);
		std::string_view name;
		std::set<std::string_view> buses;
	};
	using StopStatPtr = const StopStat*;

	struct RouteStat{
		explicit RouteStat(size_t, size_t, int64_t, double, std::string_view);
		size_t stops_on_route = 0;
		size_t unique_stops = 0;
		int64_t meters_route_length = 0;
		double curvature = 0;
		std::string name;
	};
	using RouteStatPtr = const RouteStat*;



class TransportCatalogue{
public:
		TransportCatalogue();
		~TransportCatalogue();
		void AddStop(Stop&&);
		void AddRoute(Route&&);
		void AddDistance(const Stop*, const Stop*, size_t);

		size_t GetDistance(const Stop*, const Stop*);
		size_t GetDistanceDirectly(const Stop*, const Stop*);
		const Stop* GetStopByName(const std::string_view) const;
		const Route* GetRouteByName(const std::string_view) const;

		RouteStatPtr GetRouteInfo(const std::string_view) const;
		StopStatPtr GetBusesForStopInfo(const std::string_view) const;

		void GetAllRoutes(std::map<const std::string, RendererData>&) const; 
		size_t GetAllStopsCount() const;
		const std::vector<const Stop*> GetAllStopsPtr() const;
		const std::deque<const Route*> GetAllRoutesPtr() const;
		const std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, Hasher>& GetAllDistances() const;

private:
		std::deque<Stop> all_stops_data_;
		std::unordered_map<std::string_view, const Stop*> all_stops_map_; 
		std::deque<Route> all_buses_data_;
		std::unordered_map<std::string_view, const Route*> all_buses_map_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, Hasher> distances_map_; 

		std::string_view GetStopName(const Stop* stop_ptr);
		std::string_view GetStopName(const Stop stop);
		std::string_view GetBusName(const Route* route_ptr);
		std::string_view GetBusName(const Route route);
	};
}
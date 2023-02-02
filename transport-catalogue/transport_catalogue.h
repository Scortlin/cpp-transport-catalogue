#pragma once
#include "geo.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <deque>
#include <set>

namespace Transport_Catalogue {
	namespace detail {
		struct StopFindResult {
			std::string name;
			Coordinates	coord;
			std::unordered_map<std::string, uint32_t> otherstop_dist;
		};
	}// namespace detail
	struct Stop {
		std::string name;
		Coordinates coordinates;
		std::set<std::string_view> buses;
		std::unordered_map<std::string_view, uint32_t> otherstop_dist;
	};
	struct Bus {
		std::string name;
		bool circle_key;
		std::vector<Stop*> bus_stops;
	};
	struct RouteInfo {
		size_t route_stops_count;
		size_t unique_route_stop_count;
		double route_length;
		double curvature;
	};

	class TransportCatalogue {
	public:
		void AddRoute(std::string& bus_name, const std::vector<std::string>& stops, bool circle_key);
		void AddStop(const std::string& stop_name, Coordinates& stop_coord);

		const Bus* FindRoute(const std::string& bus_name) const;
		const Stop* FindStop(const std::string& stop_name) const;

		RouteInfo Route_Info(const std::string& bus_name) const;
		std::set<std::string_view> StopInfo(const std::string& stop_name) const;

		void SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const uint32_t dist);
		double GetDistanceBetweenStops(const std::string_view from, const std::string_view to) const;

		size_t Unique_stops_count(const std::string& bus_name)const;
	private:
		std::deque<Bus> buses_;
		std::deque<Stop> busstops;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	};
}
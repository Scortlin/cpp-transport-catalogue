#include "transport_catalogue.h"

using namespace std;

namespace Transport_Catalogue {
	void TransportCatalogue::AddRoute(string& bus, const vector<string>& stops, bool circle_key) {
		Bus add_bus;
		add_bus.name = move(bus);
		add_bus.circle_key = circle_key;
		for (auto& stop : stops) {
			for (auto& busstop : busstops_) {
				if (busstop.name == stop) {
					add_bus.bus_stops.push_back(&busstop);
				}
			}
		}
		buses_.push_back(move(add_bus));
		for (auto& stop : stops) {
			for (auto& busstop : busstops_) {
				if (busstop.name == stop) {
					busstop.buses.insert(buses_.back().name);
				}
			}
		}
		busname_to_bus_[buses_.back().name] = &buses_.back();
	}
	void TransportCatalogue::AddStop(const std::string& name, Coordinates& coord) {
		busstops_.push_back({ name,move(coord),{},{} });
		stopname_to_stop_[busstops_.back().name] = &busstops_.back();
	}

	const Bus* TransportCatalogue::FindRoute(const string& bus_f) const {
		if (busname_to_bus_.count(bus_f) != 0) {
			return busname_to_bus_.at(bus_f);
		}
		return nullptr;
	}
	const Stop* TransportCatalogue::FindStop(const string& busstop_f) const {
		if (stopname_to_stop_.count(busstop_f) != 0) {
			return stopname_to_stop_.at(busstop_f);
		}
		return nullptr;
	}

	RoutInfoS TransportCatalogue::RouteInfo(const string& bus_f) const {
		RoutInfoS routinfo;
		const Bus* bus = FindRoute(bus_f);
		if (!bus) {
			throw invalid_argument("bus is not found"s);
		}
		routinfo.rout_stops_count = bus->circle_key ? (bus->bus_stops.size() + 1) : ((bus->bus_stops.size() * 2) - 1);
		routinfo.unic_rout_stop_count = Unic_stops_count(bus_f);
		double route_length = 0.0;
		double geographical_distance = 0.0;
		for (auto iter = bus->bus_stops.begin(); iter + 1 != bus->bus_stops.end(); ++iter) {
			if (bus->circle_key) {
				route_length += GetDistanceBetweenStops((*iter)->name, (*(iter + 1))->name);
				geographical_distance += ComputeDistance((*iter)->coordinates, (*(iter + 1))->coordinates);
			}
			else {
				route_length += GetDistanceBetweenStops((*iter)->name, (*(iter + 1))->name) + GetDistanceBetweenStops((*(iter + 1))->name, (*iter)->name);
				geographical_distance += ComputeDistance((*iter)->coordinates, (*(iter + 1))->coordinates) * 2;
			}
		}

		if (bus->circle_key) {
			route_length += GetDistanceBetweenStops((*(bus->bus_stops.end() - 1))->name, (*bus->bus_stops.begin())->name);
			geographical_distance += ComputeDistance((*(bus->bus_stops.end() - 1))->coordinates, (*bus->bus_stops.begin())->coordinates);
		}
		routinfo.route_length = route_length;
		routinfo.curvature = route_length / geographical_distance;
		return routinfo;
	}

	std::set<string_view> TransportCatalogue::StopInfo(const std::string& busstop) const {
		return stopname_to_stop_.at(busstop)->buses;
	}

	void TransportCatalogue::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const uint32_t dist) {
		for (auto& stop : busstops_) {
			if (from == stop.name) {
				stop.otherstop_dist[to] = dist;
				break;
			}
		}
	}

	double TransportCatalogue::GetDistanceBetweenStops(const std::string_view from, const std::string_view to) const {
		if (stopname_to_stop_.at(from)->otherstop_dist.count(to) != 0) {
			return stopname_to_stop_.at(from)->otherstop_dist.at(to);
		}
		else if (stopname_to_stop_.at(to)->otherstop_dist.count(from) != 0) {
			return stopname_to_stop_.at(to)->otherstop_dist.at(from);
		}
		return ComputeDistance(stopname_to_stop_.at(from)->coordinates, stopname_to_stop_.at(to)->coordinates);
	}
	size_t TransportCatalogue::Unic_stops_count(const string& bus_name)const {
		std::unordered_set<string_view> unic_bus_stops;
		for (const auto& bus_stop : busname_to_bus_.at(bus_name)->bus_stops) {
			unic_bus_stops.insert(bus_stop->name);
		}
		return unic_bus_stops.size();
	}
}
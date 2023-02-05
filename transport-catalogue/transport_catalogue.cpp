#include "domain.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <string_view>
#include <deque>
#include <unordered_map>
#include <algorithm>
using namespace std;
namespace transport {
	namespace catalog {
		TransportCatalogue::TransportCatalogue() {}

		void TransportCatalogue::AddStop(string_view stopName, geo::Coordinates coordinates) {
			size_t stopId = stopStorage.size();
			stopStorage.push_back(domain::Stop{ std::string(stopName), coordinates, stopId });
			stops[stopStorage.back().name] = &stopStorage.back();
			++uniqueStopCount;
		}
		void TransportCatalogue::AddRoute(string_view routeName, deque<string_view> stopsName, bool loope) {
			deque<const domain::Stop*> newStops;
			for (auto it = stopsName.begin(); it != stopsName.end(); ++it) {
				newStops.push_back(StopFind(*it));
			}
			routesStopCount += newStops.size();
			busStorage.push_back(domain::Bus{ std::string(routeName) , move(newStops), loope });
			routes[busStorage.back().name] = &busStorage.back();
		}

		void TransportCatalogue::SetDistance(string_view stopFrom, string_view stopTo, int distance) {
			if (stops.find(stopFrom) != stops.end() && stops.find(stopTo) != stops.end()) {
				distanceBwStops[{stops.at(stopFrom), stops.at(stopTo)}] = distance;
			}
		}

		const domain::Bus* TransportCatalogue::BusFind(string_view busName)const {
			return routes.at(busName);
		}

		const domain::Stop* TransportCatalogue::StopFind(string_view stopName)const {
			return stops.at(stopName);
		}

		const domain::Route TransportCatalogue::GetRoute(string_view busName) {
			try {
				const domain::Bus* busInfo = BusFind(busName);
				domain::Route result;
				result.name = busInfo->name;
				result.stops = busInfo->stops.size();
				result.length = 0;
				result.curvature = 0;
				for (auto it = busInfo->stops.begin(); it != busInfo->stops.end() - 1; ++it) {
					double curvature = geo::ComputeDistance((*it)->coord, (*(it + 1))->coord);
					double length = GetDistance(stops.at((*it)->name), stops.at((*(it + 1))->name));
					result.length += length;
					if (length == 0) {
						result.length += curvature;
					}
					result.curvature += curvature;
				}
				if (!busInfo->loope) {
					for (auto it = (busInfo->stops.end() - 1); it != busInfo->stops.begin(); --it) {
						double curvature = geo::ComputeDistance((*it)->coord, (*(it - 1))->coord);
						double length = GetDistance(stops.at((*it)->name), stops.at((*(it - 1))->name));
						result.length += length;
						if (length == 0) {
							result.length += curvature;
						}
						result.curvature += curvature;
					}
					result.stops = busInfo->stops.size() * 2 - 1;
				}
				deque<const domain::Stop*> st = busInfo->stops;
				sort(st.begin(), st.end());
				auto last = unique(st.begin(), st.end());
				result.uStops = last - st.begin();
				result.curvature = result.length / result.curvature;
				return result;
			}
			catch (...) {
				return { busName, 0, 0, 0, 0 };
			}
		}

		const deque<string_view> TransportCatalogue::GetStopBuses(string_view stopName) {
			deque<string_view> result;
			for (auto& [busname, busInfo] : routes) {
				auto findRes = find_if(busInfo->stops.begin(), busInfo->stops.end(), [stopName](const domain::Stop* stop) {
					return stop->name == stopName;
					});
				if (findRes != busInfo->stops.end()) {
					result.push_back(busname);
				}
			}
			sort(result.begin(), result.end());
			return result;
		}

		double TransportCatalogue::GetDistance(const domain::Stop* stopFrom, const domain::Stop* stopTo) {
			if (distanceBwStops.find({ stopFrom, stopTo }) != distanceBwStops.end()) {
				return distanceBwStops.at({ stopFrom, stopTo });
			}

			if (distanceBwStops.find({ stopTo, stopFrom }) != distanceBwStops.end()) {
				return distanceBwStops.at({ stopTo, stopFrom });
			}
			return 0;
		}

		deque<const domain::Bus*> TransportCatalogue::GetAllRoutes() {
			deque<const domain::Bus*> result;
			for (const auto& [name, bus] : routes) {
				if (bus->stops.size() > 0) {
					result.push_back(bus);
				}
			}
			sort(result.begin(), result.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
				return lhs->name < rhs->name; });
			return result;
		}
		size_t TransportCatalogue::GetUniqueStopCount() {
			return uniqueStopCount;
		}
	}
}
#include "request_handler.h"
#include "map_renderer.h"
#include <vector>
#include <unordered_map>
#include <string_view>
#include <deque>
#include <utility>
#include <optional>
#include <variant>
using namespace std;
namespace transport {
	namespace request {
		RequestHandler::RequestHandler(catalog::TransportCatalogue& catalog) : catalog_(catalog) {}
		void RequestHandler::CreateCatalog(const unordered_map<string_view, pair<deque<string_view>, bool>>& buses,
			const unordered_map<string_view, pair<double, double>>& stops,
			const vector<domain::DistanceBwStops>& stopsDistance) {

			for (const auto& [stopName, coord] : stops) {
				catalog_.AddStop(stopName, geo::Coordinates{ coord.first, coord.second });
			}

			for (const auto& [busName, busInfo] : buses) {
				catalog_.AddRoute(busName, busInfo.first, busInfo.second);
			}

			for (const auto& [from, to, distance] : stopsDistance) {
				catalog_.SetDistance(from, to, distance);
			}
		}
		const domain::Route RequestHandler::GetRoute(const string_view& busName) {
			return catalog_.GetRoute(busName);
		}

		optional<const deque<string_view>> RequestHandler::GetStopBuses(const string_view& stopName) {
			try {
				const domain::Stop* stop = catalog_.StopFind(stopName);
				return catalog_.GetStopBuses(stop->name);
			}
			catch (...) {
				return nullopt;
			}

		}

		void RequestHandler::SetRenderSettings(const unordered_map<string, domain::SettingType>& settings) {
			map_.SetSettings(settings);
		}

		void RequestHandler::SetRouteSettings(unordered_map<string, double>& settings) {
			route_.SetSettings(move(settings));
		}

		void RequestHandler::CreateRoute() {
			route_.CreateRoutes(catalog_);
		}

		const optional<domain::Trip>& RequestHandler::FindRoute(string_view from, string_view to) {
			const domain::Stop* fromPtr = catalog_.StopFind(from);
			const domain::Stop* toPtr = catalog_.StopFind(to);
			route_.FindRoute(fromPtr, toPtr);
			return route_.GetReadyRoute();
		}

		void RequestHandler::DrawMap(ostream& out) {
			map_.Draw(out, catalog_.GetAllRoutes());
		}
	}
}
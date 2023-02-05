#include "transport_router.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
const int MINUTES_IN_HOUR = 60;
const int METERS_IN_KILOMETR = 1000;
using namespace std;
namespace transport {
	namespace route {

		void TransportRouter::CreateWaitEdge(graph::VertexId fromId, graph::VertexId toId, string_view name, double weight) {
			auto edgeExist = graph.GetIncidentEdges(fromId);
			if (edgeExist.begin() == edgeExist.end()) {
				graph.AddEdge(graph::Edge<double> { fromId, toId, name, name, name, "Wait", weight });
			}
		}

		double TransportRouter::CalculateEdgeWeight(const transport::domain::Stop* from, const transport::domain::Stop* to, transport::catalog::TransportCatalogue& catalog) {
			double transformSpeed = settings_["bus_velocity"] * METERS_IN_KILOMETR / MINUTES_IN_HOUR;
			auto distance = catalog.GetDistance(from, to);
			return (distance / transformSpeed);
		}

		void TransportRouter::CreateRoutes(transport::catalog::TransportCatalogue& catalog) {
			size_t uniqueStopsCount = catalog.GetUniqueStopCount();
			graph::DirectedWeightedGraph<double> result(uniqueStopsCount * 2);
			graph = move(result);
			deque<const domain::Bus*> allRoutes = catalog.GetAllRoutes();
			double waitTime = settings_["bus_wait_time"];

			for (const domain::Bus* itemRoute : allRoutes) {
				for (auto itemStopIt = itemRoute->stops.begin(); itemStopIt != itemRoute->stops.end() - 1; ++itemStopIt) {
					double directWeight = 0;
					double backWeight = 0;
					int stopCount = 0;
					auto nextStopIt = itemStopIt + 1;
					pair<const transport::domain::Stop*, const transport::domain::Stop*> stopPair{ *itemStopIt , *itemStopIt };
					graph::VertexId departureVertextId = (**itemStopIt).id;
					graph::VertexId waitVertextId = (**itemStopIt).id + uniqueStopsCount;
					CreateWaitEdge(departureVertextId, waitVertextId, (**itemStopIt).name, waitTime);
					while (nextStopIt != itemRoute->stops.end()) {
						++stopCount;
						stopPair.second = *nextStopIt;
						graph::VertexId destinationVertexId = (**nextStopIt).id;
						graph::VertexId innerWaitVertexId = (**nextStopIt).id + uniqueStopsCount;
						CreateWaitEdge(destinationVertexId, innerWaitVertexId, (**nextStopIt).name, waitTime);
						directWeight += CalculateEdgeWeight(stopPair.first, stopPair.second, catalog);
						graph.AddEdge(graph::Edge<double> { waitVertextId, destinationVertexId, (**itemStopIt).name, (**nextStopIt).name, itemRoute->name, "Bus", directWeight, stopCount });
						if (!itemRoute->loope) {
							backWeight += CalculateEdgeWeight(stopPair.second, stopPair.first, catalog);
							graph.AddEdge(graph::Edge<double> { innerWaitVertexId, departureVertextId, (**nextStopIt).name, (**itemStopIt).name, itemRoute->name, "Bus", backWeight, stopCount });
						}
						stopPair.first = *nextStopIt;
						++nextStopIt;
					}
				}
			}
			routerFinder = make_unique<graph::Router<double>>(graph);
		}

		void TransportRouter::FindRoute(const domain::Stop* from, const domain::Stop* to) {
			optional<graph::Router<double>::RouteInfo> result = routerFinder->BuildRoute(from->id, to->id);
			readyRoute.reset();
			if (result.has_value()) {
				domain::Trip res;
				vector<graph::EdgeId> edgeIds = result.value().edges;
				for (const graph::EdgeId& itemEdgeId : edgeIds) {
					const graph::Edge edge = graph.GetEdge(itemEdgeId);
					res.totalTime += edge.weight;
					res.items.push_back({ edge.type, edge.weight, edge.name, edge.stopCount });
				}
				readyRoute = move(res);
			}
		}

		void TransportRouter::SetSettings(unordered_map<string, double>&& settings) {
			settings_ = move(settings);
		}
		const optional<domain::Trip>& TransportRouter::GetReadyRoute()const {
			return readyRoute;
		}
	}
}
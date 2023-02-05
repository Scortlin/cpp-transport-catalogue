#pragma once
#include "graph.h"
#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <variant>
#include <string>
#include <string_view>
#include <memory>

namespace transport {
	namespace route {
		class TransportRouter {
		private:
			graph::DirectedWeightedGraph<double> graph;
			std::unordered_map<std::string, double> settings_;
			std::optional<domain::Trip> readyRoute;
			std::unique_ptr<graph::Router<double>> routerFinder;
		public:
			TransportRouter() = default;
			const std::optional<domain::Trip>& GetReadyRoute()const;
			void SetSettings(std::unordered_map<std::string, double>&& settings);
			void CreateRoutes(transport::catalog::TransportCatalogue& catalog);
			void FindRoute(const domain::Stop* from, const domain::Stop* to);
			void CreateWaitEdge(graph::VertexId fromId, graph::VertexId toId, std::string_view name, double weight);
			double CalculateEdgeWeight(const transport::domain::Stop* from, const transport::domain::Stop* to, transport::catalog::TransportCatalogue& catalog);
		};
	}
}

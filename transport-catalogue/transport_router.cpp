#include "transport_router.h"
using namespace std;
const double MINUTES_IN_HOUR = 60.0;
const double METERS_IN_KILOMETR = 1000.0;
namespace router{

	TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& tc)
		: tc_(tc), dw_graph_(tc.GetAllStopsCount() * 2){}


	void TransportRouter::ApplyRouterSettings(RouterSettings& settings){
		settings_ = move(settings);
	}

	RouterSettings TransportRouter::GetRouterSettings() const{
		return settings_;
	}

	const RouteData TransportRouter::CalculateRoute(const string_view from, const string_view to){
		if (!router_){
			BuildGraph();
		}
		RouteData result;
		auto calculated_route = router_->BuildRoute(vertexes_wait_.at(from), vertexes_wait_.at(to));
		if (calculated_route){
			result.founded = true;
			for (const auto& element_id : calculated_route->edges){
				auto edge_details = dw_graph_.GetEdge(element_id);
				result.total_time += edge_details.weight;
				result.items.emplace_back(RouteItem{
					edge_details.edge_name,
					(edge_details.type == graph::EdgeType::TRAVEL) ? edge_details.span_count : 0,
					edge_details.weight,
					edge_details.type });
			}
		}
		return result;
	}

	void TransportRouter::BuildGraph(){
		int vertex_id = 0;
		for (const auto& stop : tc_.GetAllStopsPtr()){
			vertexes_wait_.insert({ stop->name, vertex_id });
			vertexes_travel_.insert({ stop->name, ++vertex_id });
			dw_graph_.AddEdge({
					vertexes_wait_.at(stop->name),    
					vertexes_travel_.at(stop->name),  
					settings_.bus_wait_time * 1.0, 
					stop->name,  
					graph::EdgeType::WAIT, 
					0
				});
			++vertex_id;
		}
		for (const auto& route : tc_.GetAllRoutesPtr()){
			for (size_t it_from = 0; it_from < route->stops.size() - 1; ++it_from){
				int span_count = 0;
				for (size_t it_to = it_from + 1; it_to < route->stops.size(); ++it_to){
					double road_distance = 0.0;
					for (size_t it = it_from + 1; it <= it_to; ++it)
					{
						road_distance += static_cast<double>(tc_.GetDistance(route->stops[it - 1], route->stops[it]));
					}
					dw_graph_.AddEdge({
							vertexes_travel_.at(route->stops[it_from]->name),
							vertexes_wait_.at(route->stops[it_to]->name),
							road_distance / (settings_.bus_velocity * METERS_IN_KILOMETR /  MINUTES_IN_HOUR),
							route->route_name,
							graph::EdgeType::TRAVEL,
							++span_count
						});
				}
			}
		}
		router_ = make_unique<graph::Router<double>>(dw_graph_);
	}

}
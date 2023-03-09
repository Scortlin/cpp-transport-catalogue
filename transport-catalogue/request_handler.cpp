#include "request_handler.h"
using namespace std;

namespace transport_catalogue{
	const optional<RouteStatPtr> RequestHandler::GetRouteInfo(const string_view& bus_name) const{
		return tc_.GetRouteInfo(bus_name);
	}

	const optional<StopStatPtr> RequestHandler::GetBusesForStop(const string_view& stop_name) const{
		return tc_.GetBusesForStopInfo(stop_name);
	}

	svg::Document RequestHandler::GetMapRender() const{
		map<const string, transport_catalogue::RendererData> all_routes;
		tc_.GetAllRoutes(all_routes);
		return mr_.RenderMap(all_routes);
	}
}
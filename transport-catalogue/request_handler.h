#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <unordered_set>    
#include <optional>         
#include <string_view>      
#include <map>             

namespace transport_catalogue{
    class RequestHandler{
    public:
        RequestHandler(const TransportCatalogue& tc, map_renderer::MapRenderer& mr) : tc_(tc), mr_(mr)
        {}
        const std::optional<RouteStatPtr> GetRouteInfo(const std::string_view& bus_name) const;
        const std::optional<StopStatPtr> GetBusesForStop(const std::string_view& stop_name) const;
        svg::Document GetMapRender() const;
    private:
        const TransportCatalogue& tc_;
        map_renderer::MapRenderer& mr_;
    };
}
#pragma once

#include "request_handler.h"        
#include "json_builder.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>                  
#include <sstream>                   
#include <vector>                    

namespace json_reader{
void ProcessBaseJSON(transport_catalogue::TransportCatalogue&, map_renderer::MapRenderer&, std::istream&);
void ProcessRequestJSON(transport_catalogue::TransportCatalogue&, map_renderer::MapRenderer&, std::istream&, std::ostream&);

void AddToDataBase(transport_catalogue::TransportCatalogue&, const json::Array&);
void AddStopData(transport_catalogue::TransportCatalogue&, const json::Dict&);
void AddStopDistance(transport_catalogue::TransportCatalogue&, const json::Dict&);
void AddRouteData(transport_catalogue::TransportCatalogue&, const json::Dict&);

const svg::Color ConvertJSONColorToSVG(const json::Node&);
void ReadRendererSettings(map_renderer::MapRenderer&, const json::Dict&);
void ReadRouterSettings(router::TransportRouter&, const json::Dict&);
const std::string ReadSerializationSettings(const json::Dict&);

void ParseRawJSONQueries(transport_catalogue::RequestHandler&, router::TransportRouter&, const json::Array&, std::ostream&);
const json::Node ProcessStopQuery(transport_catalogue::RequestHandler&, const json::Dict&);
const json::Node ProcessBusQuery(transport_catalogue::RequestHandler&, const json::Dict&);
const json::Node ProcessMapQuery(transport_catalogue::RequestHandler&, const json::Dict&);
const json::Node ProcessRouteQuery(router::TransportRouter&, const json::Dict&);
}
#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "graph.h"
#include "transport_catalogue.pb.h"

#include <fstream>
#include <vector>
#include <stdexcept>

namespace serialization{
	class Serializer{
	public:
		Serializer(transport_catalogue::TransportCatalogue& tc,
			map_renderer::MapRenderer& mr,
			router::TransportRouter* tr = nullptr);
		void Serialize(const std::string& filename);
		void Deserialize(const std::string& filename);
		void DeserializeRouter(router::TransportRouter* tr);
	private:
        map_renderer::MapRenderer& mr_;
		transport_catalogue::TransportCatalogue& tc_;
		router::TransportRouter* tr_ = nullptr;   
		proto_serialization::TransportCatalogue proto_all_settings_;

        void SerializeDistance();
		void SerializeRoute();
		void SerializeStop();
		proto_serialization::Color SerializeColor(const svg::Color& color);
		void SerializeRendererSettings();
		void SerializeRouterSettings();

		void DeserializeCatalogue();
		void DeserializeRenderer();
		map_renderer::RendererSettings DeserializeRendererSettings(const proto_serialization::RendererSettings& proto_renderer_settings);
		svg::Color DeserializeColor(const proto_serialization::Color& color_ser);
	};

} 
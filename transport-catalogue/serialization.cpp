#include "serialization.h"
using namespace std;
namespace serialization{
	Serializer::Serializer(transport_catalogue::TransportCatalogue& tc,
		map_renderer::MapRenderer& mr,
		router::TransportRouter* tr): tc_(tc), mr_(mr), tr_(tr){}

	void Serializer::Serialize(const string& filename){
		ofstream out(filename, ios::binary);
		proto_all_settings_.Clear();
		SerializeStop();
		SerializeDistance();
		SerializeRoute();
		SerializeRendererSettings();
		SerializeRouterSettings();
		proto_all_settings_.SerializeToOstream(&out);
	}

	void Serializer::Deserialize(const string& filename){
		std::ifstream in(filename, ios::binary);
		proto_all_settings_.Clear();
		proto_all_settings_.ParseFromIstream(&in);
		DeserializeCatalogue();
		DeserializeRenderer();
	}

	void Serializer::DeserializeRouter(router::TransportRouter* tr){
		if (tr == nullptr){
			throw runtime_error("No router object set (nullptr)");
		}
		tr_ = tr;
		const proto_serialization::RouterSettings proto_rt_settings = proto_all_settings_.router_settings();

		router::RouterSettings r_settings;
		r_settings.bus_velocity = proto_rt_settings.bus_velocity();
		r_settings.bus_wait_time = proto_rt_settings.bus_wait_time();
		tr_->ApplyRouterSettings(r_settings);
	}

	void Serializer::SerializeStop(){
		for (const auto& stop : tc_.GetAllStopsPtr()){
			proto_serialization::Stop proto_stop;
			proto_serialization::Coordinates proto_coords;
			proto_coords.set_lat(stop->coords.lat);
			proto_coords.set_lng(stop->coords.lng);
			*proto_stop.mutable_coords() = proto_coords;
			proto_stop.set_name(stop->name);
			*proto_all_settings_.add_stops() = proto_stop;
		}
	}

	void Serializer::SerializeDistance(){
		for (const auto& distance : tc_.GetAllDistances()){
			proto_serialization::Distance proto_distance;
			proto_distance.set_from(distance.first.first->name);
			proto_distance.set_to(distance.first.second->name);
			proto_distance.set_distance(distance.second);
			*proto_all_settings_.add_distances() = proto_distance;
		}
	}

	void Serializer::SerializeRoute(){
		for (const auto& route : tc_.GetAllRoutesPtr()){
			proto_serialization::Route proto_route;
			proto_route.set_route_name(route->route_name);
			proto_route.set_is_circular(route->is_circular);

			size_t num_stops_to_process = (route->is_circular ? route->stops.size() : route->stops.size() / 2 + 1);
			for (auto stop : route->stops){
				if (num_stops_to_process == 0){
					break;
				}
				--num_stops_to_process;
				proto_serialization::Stop proto_stop;
				proto_serialization::Coordinates proto_coords;
				proto_coords.set_lat(stop->coords.lat);
				proto_coords.set_lng(stop->coords.lng);
				proto_stop.set_name(stop->name);
				*proto_stop.mutable_coords() = proto_coords;
				*proto_route.add_stops() = proto_stop;
			}

			*proto_all_settings_.add_routes() = proto_route;
		}
	}

	proto_serialization::Color Serializer::SerializeColor(const svg::Color& color){
		proto_serialization::Color proto_color;
		if (holds_alternative<svg::Rgb>(color))
		{
			proto_serialization::Rgb proto_rgb;
			svg::Rgb rgb = std::get<svg::Rgb>(color);
			proto_rgb.set_red(rgb.red);
			proto_rgb.set_green(rgb.green);
			proto_rgb.set_blue(rgb.blue);
			proto_color.set_is_rgba(false);
			*proto_color.mutable_rgb() = proto_rgb;
		}
		else if (holds_alternative<svg::Rgba>(color)){
			proto_serialization::Rgba proto_rgba;
			svg::Rgba rgba = std::get<svg::Rgba>(color);
			proto_rgba.set_red(rgba.red);
			proto_rgba.set_green(rgba.green);
			proto_rgba.set_blue(rgba.blue);
			proto_rgba.set_opacity(rgba.opacity);
			proto_color.set_is_rgba(true);
			*proto_color.mutable_rgba() = proto_rgba;
		}
		else if (holds_alternative<string>(color)){
			*proto_color.mutable_name() = get<string>(color);
		}
		return proto_color;
	}

	void Serializer::SerializeRendererSettings(){
		const auto renderer_settings = mr_.GetRendererSettings();
		proto_serialization::RendererSettings proto_renderer_settings;
		proto_serialization::Point proto_point;

		proto_renderer_settings.set_width(renderer_settings.width);
		proto_renderer_settings.set_height(renderer_settings.height);
		proto_renderer_settings.set_padding(renderer_settings.padding);
		proto_renderer_settings.set_line_width(renderer_settings.line_width);
		proto_renderer_settings.set_stop_radius(renderer_settings.stop_radius);
		proto_renderer_settings.set_bus_label_font_size(renderer_settings.bus_label_font_size);
		proto_renderer_settings.set_stop_label_font_size(renderer_settings.stop_label_font_size);
		proto_renderer_settings.set_underlayer_width(renderer_settings.underlayer_width);

		proto_point.set_x(renderer_settings.bus_label_offset.x);
		proto_point.set_y(renderer_settings.bus_label_offset.y);
		*proto_renderer_settings.mutable_bus_label_offset() = proto_point;

		proto_point.Clear();
		proto_point.set_x(renderer_settings.stop_label_offset.x);
		proto_point.set_y(renderer_settings.stop_label_offset.y);
		*proto_renderer_settings.mutable_stop_label_offset() = proto_point;
		*proto_renderer_settings.mutable_underlayer_color() = SerializeColor(renderer_settings.underlayer_color);

		for (const auto& color : renderer_settings.color_palette){
			*proto_renderer_settings.add_color_palette() = SerializeColor(color);
		}
		*proto_all_settings_.mutable_renderer_settings() = proto_renderer_settings;
	}


	void Serializer::SerializeRouterSettings(){
		proto_serialization::RouterSettings proto_router_settings;
		const auto rt_settings = tr_->GetRouterSettings();

		proto_router_settings.set_bus_velocity(rt_settings.bus_velocity);
		proto_router_settings.set_bus_wait_time(rt_settings.bus_wait_time);

		*proto_all_settings_.mutable_router_settings() = proto_router_settings;
	}

	void Serializer::DeserializeCatalogue(){
		for (int i = 0; i < proto_all_settings_.stops_size(); ++i){
			proto_serialization::Stop proto_stop = proto_all_settings_.stops(i);
			tc_.AddStop(transport_catalogue::Stop(proto_stop.name(), proto_stop.coords().lat(), proto_stop.coords().lng()));
		}
		for (int i = 0; i < proto_all_settings_.distances_size(); ++i){
			const transport_catalogue::Stop* stop_from_ptr = tc_.GetStopByName(proto_all_settings_.distances(i).from());
			const transport_catalogue::Stop* stop_to_ptr = tc_.GetStopByName(proto_all_settings_.distances(i).to());
			size_t distance = proto_all_settings_.distances(i).distance();
			tc_.AddDistance(stop_from_ptr, stop_to_ptr, distance);
		}

		for (int i = 0; i < proto_all_settings_.routes_size(); ++i){
			proto_serialization::Route proto_route = proto_all_settings_.routes(i);
			transport_catalogue::Route route;
			route.route_name = proto_route.route_name();
			route.is_circular = proto_route.is_circular();

			for (const auto& proto_stop : proto_route.stops()){
				route.stops.push_back(tc_.GetStopByName(proto_stop.name()));
			}
			tc_.AddRoute(std::move(route));
		}
	}

	void Serializer::DeserializeRenderer(){
     mr_.ApplyRendererSettings(DeserializeRendererSettings(proto_all_settings_.renderer_settings()));
	}

	map_renderer::RendererSettings Serializer::DeserializeRendererSettings(const proto_serialization::RendererSettings& proto_renderer_settings){
		map_renderer::RendererSettings renderer_settings;
		renderer_settings.color_palette.clear();
		renderer_settings.width = proto_renderer_settings.width();
		renderer_settings.height = proto_renderer_settings.height();
		renderer_settings.padding = proto_renderer_settings.padding();
		renderer_settings.line_width = proto_renderer_settings.line_width();
		renderer_settings.stop_radius = proto_renderer_settings.stop_radius();
		renderer_settings.bus_label_font_size = proto_renderer_settings.bus_label_font_size();
		renderer_settings.stop_label_font_size = proto_renderer_settings.stop_label_font_size();
		renderer_settings.underlayer_width = proto_renderer_settings.underlayer_width();

		renderer_settings.bus_label_offset = { proto_renderer_settings.bus_label_offset().x(), proto_renderer_settings.bus_label_offset().y() };
		renderer_settings.stop_label_offset = { proto_renderer_settings.stop_label_offset().x(), proto_renderer_settings.stop_label_offset().y() };

		renderer_settings.underlayer_color = DeserializeColor(proto_renderer_settings.underlayer_color());

		for (const auto& proto_color : proto_renderer_settings.color_palette()){
			renderer_settings.color_palette.push_back(DeserializeColor(proto_color));
		}
		return renderer_settings;
	}

	svg::Color Serializer::DeserializeColor(const proto_serialization::Color& proto_color){
		if (!proto_color.name().empty()){
            return svg::Color{ proto_color.name() };
		}
		else if (proto_color.is_rgba()){
			return svg::Rgba(proto_color.rgba().red(), proto_color.rgba().green(), proto_color.rgba().blue(), proto_color.rgba().opacity());
		}
		return svg::Rgb(proto_color.rgb().red(), proto_color.rgb().green(), proto_color.rgb().blue());
	}
}
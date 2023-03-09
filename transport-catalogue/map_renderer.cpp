#include "map_renderer.h"
using namespace std;
namespace map_renderer{
	bool IsZero(const double value){
		return (std::abs(value) < EPSILON);
	}

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const{
		return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
	}

	RouteLine::RouteLine(const vector<svg::Point>& stop_points,
		const svg::Color& stroke_color,
		const RendererSettings& renderer_settings) :
		stop_points_(stop_points), stroke_color_(stroke_color), renderer_settings_(renderer_settings)
	{}

	void RouteLine::Draw(svg::ObjectContainer& container) const{
		svg::Polyline polyline;
		for (const auto& point : stop_points_)
		{
			polyline.AddPoint(point);
		}
		polyline.SetStrokeColor(stroke_color_);
		polyline.SetFillColor(svg::NoneColor);
		polyline.SetStrokeWidth(renderer_settings_.line_width);
		polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		container.Add(polyline);
	}

	TextLabel::TextLabel(const svg::Point& label_point,
		const string& text,
		const svg::Color& fill_fore_color,
		const RendererSettings& renderer_settings,
		const bool& is_stop) :
		label_point_(label_point), text_(text), fill_fore_color_(fill_fore_color),
		renderer_settings_(renderer_settings), is_stop_(is_stop)
	{}

	void TextLabel::Draw(svg::ObjectContainer& container) const{
		svg::Text fore_text;
		fore_text.SetPosition(label_point_);
		fore_text.SetFontFamily("Verdana"s);
		fore_text.SetData(text_);
		if (is_stop_){
			fore_text.SetOffset(renderer_settings_.stop_label_offset);
			fore_text.SetFontSize(renderer_settings_.stop_label_font_size);
			fore_text.SetFillColor("black"s);
		}
		else{
			fore_text.SetOffset(renderer_settings_.bus_label_offset);
			fore_text.SetFontSize(renderer_settings_.bus_label_font_size);
			fore_text.SetFontWeight("bold"s);
			fore_text.SetFillColor(fill_fore_color_);
		}

		svg::Text back_text = fore_text;
		back_text.SetFillColor(renderer_settings_.underlayer_color);
		back_text.SetStrokeColor(renderer_settings_.underlayer_color);
		back_text.SetStrokeWidth(renderer_settings_.underlayer_width);
		back_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		back_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		container.Add(back_text);
		container.Add(fore_text);
	}

	StopIcon::StopIcon(const svg::Point& label_point, const RendererSettings& renderer_settings) :
		label_point_(label_point), renderer_settings_(renderer_settings)
	{}

	void StopIcon::Draw(svg::ObjectContainer& container) const{
		svg::Circle icon;
		icon.SetCenter(label_point_);
		icon.SetRadius(renderer_settings_.stop_radius);
		icon.SetFillColor("white"s);
		container.Add(icon);
	}

	void MapRenderer::ApplyRendererSettings(RendererSettings settings){
		settings_ = settings;
	}

	RendererSettings MapRenderer::GetRendererSettings() const{
		return settings_;
	}

	void MapRenderer::AddRouteLinesToRender(vector<unique_ptr<svg::Drawable>>& picture_,
		SphereProjector& sp,
		map<const string, transport_catalogue::RendererData>& routes_to_render){
		for (const auto& [name, data] : routes_to_render){
			std::vector<svg::Point> points;
			for (const auto& stop : data.stop_coords){
				points.push_back(sp(stop));
			}
			picture_.emplace_back(std::make_unique<RouteLine>(RouteLine{ points, GetColorFromPallete() , settings_ }));
		}
	}

	void MapRenderer::AddRouteLabelsToRender(vector<unique_ptr<svg::Drawable>>& picture_,
		SphereProjector& sp,
		map<const string, transport_catalogue::RendererData>& routes_to_render){
		ResetPallette();
		for (const auto& [name, data] : routes_to_render){
			svg::Color current_line_color = GetColorFromPallete();
			picture_.emplace_back(make_unique<TextLabel>(TextLabel{ sp(data.stop_coords[0]),
															  name,
															  current_line_color,
															  settings_,
															  false }));
			if ((!data.is_circular) && (data.stop_coords.size() > 1)){
				if (data.stop_coords[0] != data.stop_coords[(data.stop_coords.size() + 1) / 2 - 1]){
					picture_.emplace_back(std::make_unique<TextLabel>(TextLabel{ sp(data.stop_coords[(data.stop_coords.size() + 1) / 2 - 1]),name,current_line_color,settings_,false }));
				}
			}
		}
	}

	void MapRenderer::AddStopLabelsToRender(vector<unique_ptr<svg::Drawable>>& picture_,
		SphereProjector& sp,
		map<string_view, geo::Coordinates> all_unique_stops){
		for (const auto& stop : all_unique_stops){
			picture_.emplace_back(make_unique<StopIcon>(StopIcon{ sp(stop.second), settings_ }));
		}
	}

	void MapRenderer::AddStopIconsToRender(vector<unique_ptr<svg::Drawable>>& picture_,
		SphereProjector& sp,
		map<string_view, geo::Coordinates> all_unique_stops){
		for (const auto& stop : all_unique_stops){
			picture_.emplace_back(make_unique<TextLabel>(TextLabel{sp(stop.second),string(stop.first),"black"s,settings_,true }));
		}
	}

	svg::Document MapRenderer::RenderMap(map<const string, transport_catalogue::RendererData>& routes_to_render){
		unordered_set<geo::Coordinates, geo::CoordinatesHasher> all_coords;
		map<std::string_view, geo::Coordinates> all_unique_stops;
		for (const auto& [name, data] : routes_to_render){
			for (size_t i = 0; i < data.stop_coords.size(); ++i){
				all_coords.insert(data.stop_coords[i]);
				all_unique_stops.insert(make_pair(data.stop_names[i], data.stop_coords[i]));
			}
		}
		SphereProjector sp{begin(all_coords),end(all_coords),
			settings_.width, settings_.height, settings_.padding };

		vector<unique_ptr<svg::Drawable>> picture_;   
		AddRouteLinesToRender(picture_, sp, routes_to_render);
		AddRouteLabelsToRender(picture_, sp, routes_to_render);
		AddStopLabelsToRender(picture_, sp, all_unique_stops);
		AddStopIconsToRender(picture_, sp, all_unique_stops);
		svg::Document map;
		DrawPicture(picture_, map);
		return map;
	}

	const svg::Color MapRenderer::GetColorFromPallete(){
		if (pallette_item_ == settings_.color_palette.size()){
			pallette_item_ = 0;
		}
		return settings_.color_palette[pallette_item_++];
	}

	void MapRenderer::ResetPallette(){
		pallette_item_ = 0;
	}
}
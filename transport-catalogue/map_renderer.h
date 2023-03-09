#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"
#include <unordered_set>  
#include <algorithm>      
#include <memory> 
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include <map>            
        
namespace map_renderer{
    struct RendererSettings{
        double width = 1200.0;
        double height = 1200.0;
        double padding = 50.0;
        double line_width = 14.0;
        double stop_radius = 5.0;
        int bus_label_font_size = 20;
        svg::Point bus_label_offset{ 7.0, 15.0 };
        int stop_label_font_size = 20;
        svg::Point stop_label_offset{ 7.0, -3.0 };
        svg::Color underlayer_color = svg::Rgba{ 255, 255, 255, 0.85 };
        double underlayer_width = 3.0;
        std::vector<svg::Color> color_palette{ std::string("green"), svg::Rgb{255, 160, 0}, std::string("red") };
    };
    inline const double EPSILON = 1e-6;

    bool IsZero(const double);

    class SphereProjector{
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,double max_width,
          double max_height,double padding): padding_(padding){
            if (points_begin == points_end){
                return;
            }
            const auto [left_it, right_it]= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs){return lhs.lng < rhs.lng;});
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs)
                    {
                        return lhs.lat < rhs.lat;
                    });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)){
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)){
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }
            if (width_zoom && height_zoom){
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom){
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom){
                zoom_coeff_ = *height_zoom;
            }
        }
        svg::Point operator()(geo::Coordinates) const;
    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class RouteLine : public svg::Drawable{
    public:
        RouteLine(const std::vector<svg::Point>&, const svg::Color&, const RendererSettings&);
        void Draw(svg::ObjectContainer&) const override;
    private:
        std::vector<svg::Point> stop_points_;
        svg::Color stroke_color_;
        const RendererSettings& renderer_settings_;
    };

    class TextLabel : public svg::Drawable{
    public:
        TextLabel(const svg::Point&, const std::string& text, const svg::Color&, const RendererSettings&, const bool& is_stop);
        void Draw(svg::ObjectContainer&) const override;
    private:
        svg::Point label_point_;
        std::string font_family_;
        std::string font_weight_;
        std::string text_;
        svg::Color fill_fore_color_;
        const RendererSettings& renderer_settings_;
        bool is_stop_;
    };

    class StopIcon : public svg::Drawable{
    public:
        StopIcon(const svg::Point&, const RendererSettings&);
        void Draw(svg::ObjectContainer&) const override;
    private:
        svg::Point label_point_;
        const RendererSettings& renderer_settings_;
    };

    class MapRenderer{
    public:
        void ApplyRendererSettings(RendererSettings);
        RendererSettings GetRendererSettings() const;
        void AddRouteLinesToRender(std::vector<std::unique_ptr<svg::Drawable>>& picture_,
            SphereProjector& sp,
            std::map<const std::string, transport_catalogue::RendererData>& routes_to_render);
        void AddRouteLabelsToRender(std::vector<std::unique_ptr<svg::Drawable>>& picture_,
            SphereProjector& sp,
            std::map<const std::string, transport_catalogue::RendererData>& routes_to_render);
        void AddStopLabelsToRender(std::vector<std::unique_ptr<svg::Drawable>>& picture_,
            SphereProjector& sp,
            std::map<std::string_view, geo::Coordinates> all_unique_stops);
        void AddStopIconsToRender(std::vector<std::unique_ptr<svg::Drawable>>& picture_,
            SphereProjector& sp,
            std::map<std::string_view, geo::Coordinates> all_unique_stops);

        svg::Document RenderMap(std::map<const std::string, transport_catalogue::RendererData>&);

        template <typename DrawableIterator>
        void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target){
            for (auto it = begin; it != end; ++it){
                (*it)->Draw(target);
            }
        }
        template <typename Container>
        void DrawPicture(const Container& container, svg::ObjectContainer& target){
            DrawPicture(begin(container), end(container), target);
        }
    private:
        RendererSettings settings_;
        size_t pallette_item_ = 0; 
        const svg::Color GetColorFromPallete();
        void ResetPallette();
    };
}
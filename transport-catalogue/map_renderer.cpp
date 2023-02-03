#include "svg.h"
#include "domain.h"
#include "map_renderer.h"

#include <algorithm>
#include <variant>
#include <cctype>
#include <iostream>
#include <deque>
#include <vector>
#include <unordered_set>
#include <set>
using namespace std;
namespace transport {
    namespace render {
        void MapRenderer::SetSettings(std::unordered_map<std::string, domain::SettingType> settings) {
            settings_ = std::move(settings);
        }

        const set<const domain::Stop*, domain::StopCompare> MapRenderer::GetAllStops(const deque<const domain::Bus*>& routes, unordered_set<geo::Coordinates, geo::Hasher>& allCoord) {
            set<const domain::Stop*, domain::StopCompare> result;
            for (const domain::Bus* bus : routes) {
                for (const domain::Stop* stop : bus->stops) {
                    allCoord.insert(stop->coord);
                    result.insert(stop);
                }
            }
            return result;
        }

        pair<svg::Text, svg::Text> MapRenderer::DrawText(string name, svg::Point point, svg::Color color, bool isBus) {
            double strokeWidth = std::get<double>(settings_["underlayer_width"]);
            int fontSize;
            pair<double, double> offset;
            if (isBus) {
                offset = get<std::pair<double, double>>(settings_["bus_label_offset"]);
                fontSize = get<double>(settings_["bus_label_font_size"]);
            }
            else {
                offset = get<std::pair<double, double>>(settings_["stop_label_offset"]);
                fontSize = get<double>(settings_["stop_label_font_size"]);
            }
            svg::Color textColor = get<svg::Color>(settings_["underlayer_color"]);
            svg::Text underText;
            svg::Text text;
            underText.SetFillColor(textColor)
                .SetPosition(point)
                .SetOffset(svg::Point{ offset.first, offset.second })
                .SetStrokeColor(textColor)
                .SetStrokeWidth(strokeWidth)
                .SetFontSize(fontSize)
                .SetFontFamily("Verdana")
                .SetData(name)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            text.SetFillColor(color)
                .SetPosition(point)
                .SetOffset(svg::Point{ offset.first, offset.second })
                .SetFontSize(fontSize)
                .SetFontFamily("Verdana")
                .SetData(name);
            if (isBus) {
                underText.SetFontWeight("bold");
                text.SetFontWeight("bold");
            }
            return { underText , text };
        }

        svg::Circle MapRenderer::DrawCircle(svg::Point point) {
            svg::Circle circle;
            double radius = get<double>(settings_["stop_radius"]);
            circle.SetCenter(point).SetRadius(radius).SetFillColor("white");
            return circle;
        }

        void MapRenderer::Draw(ostream& out, deque<const domain::Bus*> routes) {
            unordered_set<geo::Coordinates, geo::Hasher> allCoord;
            const set<const domain::Stop*, domain::StopCompare> allStops = GetAllStops(routes, allCoord);
            double width = get<double>(settings_["width"]);
            double height = get<double>(settings_["height"]);
            double padding = get<double>(settings_["padding"]);
            double lineWidth = get<double>(settings_["line_width"]);

            vector<svg::Color> colorPalette = get<vector<svg::Color>>(settings_["color_palette"]);

            const SphereProjector proj{ allCoord.begin(), allCoord.end(), width, height, padding };
            svg::Document doc;
            size_t colorIndex = 0;
            vector<pair<svg::Text, svg::Text>> texts;
            vector<svg::Circle> circles;
            for (const domain::Bus* bus : routes) {
                if (colorIndex >= colorPalette.size()) {
                    colorIndex = 0;
                }
                svg::Polyline route;
                geo::Coordinates firstPoint = bus->stops[0]->coord;
                geo::Coordinates lastPoint = bus->stops[bus->stops.size() - 1]->coord;
                texts.push_back(DrawText(bus->name, proj(firstPoint), colorPalette[colorIndex]));
                if (!bus->loope && firstPoint != lastPoint) {
                    texts.push_back(DrawText(bus->name, proj(lastPoint), colorPalette[colorIndex]));
                }
                for (size_t i = 0; i < bus->stops.size(); ++i) {
                    svg::Point stopPoint = proj(bus->stops[i]->coord);
                    route.AddPoint(stopPoint);
                }
                if (!bus->loope && bus->stops.size() >= 2) {
                    for (int i = (bus->stops.size() - 2); i >= 0; --i) {
                        route.AddPoint(proj(bus->stops[i]->coord));
                    }
                }
                route.SetStrokeColor(colorPalette[colorIndex])
                    .SetStrokeWidth(lineWidth)
                    .SetFillColor("none")
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                doc.Add(route);
                ++colorIndex;
            }

            for (const auto& [underText, text] : texts) {
                doc.Add(underText);
                doc.Add(text);
            }

            for (const domain::Stop* stop : allStops) {
                svg::Point stopPoint = proj(stop->coord);
                doc.Add(DrawCircle(stopPoint));
            }

            for (const domain::Stop* stop : allStops) {
                svg::Point stopPoint = proj(stop->coord);
                pair<svg::Text, svg::Text> texts = DrawText(stop->name, stopPoint, svg::Color{ "black" }, false);
                doc.Add(texts.first);
                doc.Add(texts.second);
            }
            doc.Render(out);
        }
    }
}
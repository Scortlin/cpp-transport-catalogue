#include "json_reader.h"
#include "svg.h"
#include "json.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <string_view>
#include <utility>
#include <optional>
#include <variant>
#include <sstream>
#include <execution>
#include <string>
#include <stack>
#include <vector>
#include "json.h"
using namespace std;
namespace json {
	class KeyItemContext;
	class SingleValueItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
	private:
		Node root_;
		stack<Node*> queue_;
	public:
		Builder() = default;
		KeyItemContext Key(string key);
		SingleValueItemContext Value(Node::Value value);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node Build();
	};

	class ItemContext {
	protected:
		Builder& builder_;
	public:
		ItemContext(Builder& builder);
		KeyItemContext Key(string key);
		SingleValueItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();
	};

	class KeyItemContext : public ItemContext {
	public:
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value value);
		KeyItemContext Key(string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class SingleValueItemContext : public ItemContext {
	public:
		SingleValueItemContext(Builder& builder);
		KeyItemContext Key(string key) = delete;
		SingleValueItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndDict() = delete;
	};

	class DictItemContext : public ItemContext {
	public:
		DictItemContext(Builder& builder);
		SingleValueItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class ArrayItemContext : public ItemContext {
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value value);
		KeyItemContext Key(string key) = delete;
		Builder& EndDict() = delete;
		Node Build() = delete;
	};
}
namespace json {

	KeyItemContext Builder::Key(std::string key) {
		if (queue_.empty() || !queue_.top()->IsMap()) {
			throw logic_error("Dictonary not found, or value expected");
		}
		queue_.push(new Node(move(key)));
		return { *this };
	}

	SingleValueItemContext Builder::Value(Node::Value value) {
		if (queue_.empty() && root_.GetValue().index() != 0) {
			throw logic_error("Object already ready");
		}

		if (queue_.empty()) {
			root_.Swap(move(value));
		}
		else {
			Node* last = queue_.top();
			if (last->IsArray()) {
				last->AddValue(move(value));
			}
			else {
				string key = last->AsString();
				queue_.pop();
				queue_.top()->AddValue(last->AsString(), std::move(value));
			}
		}
		return { *this };
	}

	DictItemContext Builder::StartDict() {
		if (!queue_.empty() && queue_.top()->IsMap()) {
			throw logic_error("Dictonary key expected");
		}
		queue_.push(new Node(Dict{}));
		return { *this };
	}

	Builder& Builder::EndDict() {
		if (queue_.empty() || !queue_.top()->IsMap()) {
			throw logic_error("Array not found");
		}

		Node top = std::move(*(queue_.top()));
		queue_.pop();

		if (queue_.empty()) {
			root_ = std::move(top);
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsArray()) {
				parent->AddValue(top.AsMap());
			}
			else {
				string key = parent->AsString();
				queue_.pop();
				queue_.top()->AddValue(key, top.AsMap());
			}
		}
		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		queue_.push(new Node(Array{}));
		return { *this };
	}

	Builder& Builder::EndArray() {
		if (queue_.empty() || !queue_.top()->IsArray()) {
			throw logic_error("Array not found");
		}
		Node top = move(*(queue_.top()));
		queue_.pop();
		if (queue_.empty()) {
			root_ = std::move(top);
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsArray()) {
				parent->AddValue(top.AsArray());
			}
			else {
				string key = parent->AsString();
				queue_.pop();
				queue_.top()->AddValue(key, top.AsArray());
			}
		}
		return *this;
	}

	Node Builder::Build() {
		if (!queue_.empty() || root_.GetValue().index() == 0) {
			throw logic_error("Object not ready");
		}
		return root_;
	}

	ItemContext::ItemContext(Builder& builder) : builder_(builder) {}
	KeyItemContext ItemContext::Key(string key) {
		return builder_.Key(key);
	}

	SingleValueItemContext ItemContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

	DictItemContext ItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ItemContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& ItemContext::EndArray() {
		return builder_.EndArray();
	}

	Node ItemContext::Build() {
		return builder_.Build();
	}

	KeyItemContext::KeyItemContext(Builder& builder) : ItemContext(builder) {}
	SingleValueItemContext::SingleValueItemContext(Builder& builder) : ItemContext(builder) {}
	DictItemContext::DictItemContext(Builder& builder) : ItemContext(builder) {}
	ArrayItemContext::ArrayItemContext(Builder& builder) : ItemContext(builder) {}

	DictItemContext KeyItemContext::Value(Node::Value value) {
		ItemContext::Value(value);
		return { builder_ };
	}

	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		ItemContext::Value(value);
		return { builder_ };
	}
}
using namespace std::string_literals;

namespace transport {
	namespace json_reader {
		JsonReader::JsonReader(request::RequestHandler& handler, istream& input) : handler_(handler), input_(input) {
			HandleStream();
		}

		void JsonReader::HandleStream() {
			json::Document jsonObject = json::Load(input_);
			data_ = jsonObject.GetRoot().AsMap().at("base_requests");
			query_ = jsonObject.GetRoot().AsMap().at("stat_requests");
			try {
				PrepareSettings(jsonObject.GetRoot().AsMap().at("render_settings"));
			}
			catch (...) {}
		}

		void JsonReader::PrepareSettings(const json::Node jsonSettings) {
			unordered_map<string, domain::SettingType> settings;

			for (const auto& [key, value] : jsonSettings.AsMap()) {
				if (key == "color_palette") {
					vector<svg::Color> colorPalette;
					for (const json::Node& itemColor : value.AsArray()) {
						if (itemColor.IsString()) {
							colorPalette.push_back(itemColor.AsString());
							continue;
						}
						if (itemColor.IsArray()) {
							json::Array colors = itemColor.AsArray();
							if (colors.size() == 3) {
								colorPalette.push_back(svg::Color{ svg::Rgb{ static_cast<uint8_t>(colors[0].AsInt()),  static_cast<uint8_t>(colors[1].AsInt()),  static_cast<uint8_t>(colors[2].AsInt())} });
							}
							else if (colors.size() == 4) {
								colorPalette.push_back(svg::Color{ svg::Rgba{ static_cast<uint8_t>(colors[0].AsInt()),  static_cast<uint8_t>(colors[1].AsInt()),  static_cast<uint8_t>(colors[2].AsInt()), colors[3].AsDouble() } });
							}
						}

					}
					settings["color_palette"] = colorPalette;
				}
				if (key == "underlayer_color") {
					if (value.IsString()) {
						settings["underlayer_color"] = svg::Color{ value.AsString() };
					}
					else if (value.IsArray()) {
						json::Array underlayerColor = value.AsArray();
						if (underlayerColor.size() == 3) {
							settings["underlayer_color"] = svg::Color{ svg::Rgb{ static_cast<uint8_t>(underlayerColor[0].AsInt()),  static_cast<uint8_t>(underlayerColor[1].AsInt()),  static_cast<uint8_t>(underlayerColor[2].AsInt())} };
						}
						else {
							settings["underlayer_color"] = svg::Color{ svg::Rgba{ static_cast<uint8_t>(underlayerColor[0].AsInt()),  static_cast<uint8_t>(underlayerColor[1].AsInt()),  static_cast<uint8_t>(underlayerColor[2].AsInt()), underlayerColor[3].AsDouble()} };
						}
					}
				}
				if (key == "bus_label_offset" || key == "stop_label_offset") {
					if (value.IsArray()) {
						if (value.AsArray().front().IsDouble() && value.AsArray().back().IsDouble()) {
							settings[key] = pair<double, double>{ value.AsArray().front().AsDouble(), value.AsArray().back().AsDouble() };
						}
					}
				}
				if (value.IsDouble()) {
					settings[key] = value.AsDouble();
				}
			}
			handler_.SetRenderSettings(settings);
		}

		void JsonReader::HandleDataBase() {
			for (const json::Node& item : data_->AsArray()) {
				const string& type = item.AsMap().at("type").AsString();
				const string& name = item.AsMap().at("name").AsString();
				if (type == "Stop") {
					double latitude = item.AsMap().at("latitude").AsDouble();
					double longtitude = item.AsMap().at("longitude").AsDouble();
					stops_[name] = { latitude, longtitude };
					try {
						const json::Dict& roadDistance = item.AsMap().at("road_distances").AsMap();
						for (const auto& [toStop, dictance] : roadDistance) {
							stopsDistance_.push_back(domain::DistanceBwStops{ name, toStop,  dictance.AsInt() });
						}
					}
					catch (...) {}
				}
				else {
					deque<string_view> busStops;
					for (const json::Node& busItemStop : item.AsMap().at("stops").AsArray()) {
						busStops.push_back(busItemStop.AsString());
					}
					buses_[name] = { busStops, item.AsMap().at("is_roundtrip").AsBool() };
				}
			}
			handler_.CreateCatalog(buses_, stops_, stopsDistance_);
		}

		void JsonReader::HandleStopQuery(const json::Node& stop, json::Array& saveConatiner) {
			int id = stop.AsMap().at("id").AsInt();
			string_view name = stop.AsMap().at("name").AsString();
			optional<const deque<string_view>> stopBus = handler_.GetStopBuses(name);
			if (stopBus.has_value()) {
				if (stopBus.value().empty()) {
					saveConatiner.push_back(json::Builder{}
						.StartDict()
						.Key("request_id"s)
						.Value(id)
						.Key("buses")
						.Value(json::Array{})
						.EndDict()
						.Build());
				}
				else {
					saveConatiner.push_back(json::Builder{}
						.StartDict()
						.Key("request_id"s)
						.Value(id)
						.Key("buses")
						.Value(json::Array{ stopBus.value().begin(), stopBus.value().end() })
						.EndDict()
						.Build());
				}
			}
			else {
				saveConatiner.push_back(json::Builder{}
					.StartDict()
					.Key("request_id"s)
					.Value(id)
					.Key("error_message"s)
					.Value("not found"s)
					.EndDict()
					.Build());
			}
		}

		void JsonReader::HandleBusQuery(const json::Node& bus, json::Array& saveConatiner) {
			int id = bus.AsMap().at("id").AsInt();
			string_view name = bus.AsMap().at("name").AsString();
			const domain::Route route = handler_.GetRoute(name);
			if (route.stops == 0) {
				saveConatiner.push_back(json::Builder{}
					.StartDict()
					.Key("request_id"s)
					.Value(id)
					.Key("error_message"s)
					.Value("not found"s)
					.EndDict()
					.Build());
			}
			else {
				saveConatiner.push_back(json::Builder{}
					.StartDict()
					.Key("request_id"s)
					.Value(id)
					.Key("curvature"s)
					.Value(route.curvature)
					.Key("route_length"s)
					.Value(route.length)
					.Key("stop_count"s)
					.Value(static_cast<int>(route.stops))
					.Key("unique_stop_count"s)
					.Value(static_cast<int>(route.uStops))
					.EndDict()
					.Build());
			}
		}

		void JsonReader::HandleMapQuery(const json::Node& map, json::Array& saveConatiner) {
			int id = map.AsMap().at("id").AsInt();
			ostringstream out;
			handler_.DrawMap(out);
			saveConatiner.push_back(json::Builder{}
				.StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("map")
				.Value(out.str())
				.EndDict()
				.Build());
		}

		void JsonReader::HandleQuery() {
			json::Array result;
			for (const json::Node& item : query_->AsArray()) {
				string_view type = item.AsMap().at("type").AsString();
				if (type == "Stop") {
					HandleStopQuery(item, result);
				}
				else if (type == "Bus") {
					HandleBusQuery(item, result);
				}
				else if (type == "Map") {
					HandleMapQuery(item, result);
				}
			}
			nodeResul_ = json::Node(result);
		}

		void JsonReader::Print(std::ostream& output) {
			json::PrintNode(nodeResul_, output);
		}
	}
}
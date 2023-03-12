#include <cassert>
#include <string>
#include <utility>
#include <variant>

#include "json_reader.h"

using namespace std::literals;

namespace io {

	// ---------------------- Input ----------------------

	JsonReader::JsonReader(std::istream& input)
		: input_(input) {
	}

	Input JsonReader::Read() {
		Input input;

		const auto document = json::Load(input_).GetRoot().AsMap();
		
		for (const auto& base_request_node : document.at("base_requests"s).AsArray()) {
			const auto& base_request = base_request_node.AsMap();			
			const auto& request_type = base_request.at("type"s).AsString();
			if (request_type == "Bus"s) {
				input.buses.push_back(ReadBaseRequestBus(base_request));
			}
			else if (request_type == "Stop"s) {
				input.stops.push_back(ReadBaseRequestStop(base_request));
			}
			else {
				assert(false);
			}
		}

		for (const auto& stat_request_node : document.at("stat_requests"s).AsArray()) {
			const auto& stat_request = stat_request_node.AsMap();
			input.stat_requests.push_back(ReadStatRequest(stat_request));
		}

		input.render_settings = ReadRenderSettings(document.at("render_settings"s).AsMap());

		return input;
	}

	BaseRequestBus JsonReader::ReadBaseRequestBus(const json::Dict& base_request) {
		BaseRequestBus bus;

		bus.name = base_request.at("name"s).AsString();
		bus.ring = base_request.at("is_roundtrip"s).AsBool();

		std::vector<std::string> bus_stops;
		for (const auto& stop : base_request.at("stops"s).AsArray()) {
			bus_stops.push_back(stop.AsString());
		}
		bus.stops = std::move(bus_stops);

		return bus;
	}

	BaseRequestStop JsonReader::ReadBaseRequestStop(const json::Dict& base_request) {
		BaseRequestStop stop;

		stop.name = base_request.at("name"s).AsString();
		stop.coordinates.lat = base_request.at("latitude"s).AsDouble();
		stop.coordinates.lng = base_request.at("longitude"s).AsDouble();

		std::vector<std::pair<std::string, uint32_t>> distances;
		for (const auto& [to, distance] : base_request.at("road_distances"s).AsMap()) {
			distances.emplace_back(to, distance.AsInt());
		}
		stop.distances = std::move(distances);

		return stop;
	}

	StatRequest JsonReader::ReadStatRequest(const json::Dict& stat_request) {
		StatRequest request;

		request.id = stat_request.at("id"s).AsInt();

		const auto& stat_request_type = stat_request.at("type"s).AsString();
		if (stat_request_type == "Bus"s) {
			request.type = StatRequest::Type::BUS;
		}
		else if (stat_request_type == "Stop"s) {
			request.type = StatRequest::Type::STOP;
		}
		else if (stat_request_type == "Map"s) {
			request.type = StatRequest::Type::MAP;
		}
		else {
			assert(false);
		}

		if (stat_request_type != "Map"s) {
			request.name = stat_request.at("name"s).AsString();
		}

		return request;
	}

	RenderSettings JsonReader::ReadRenderSettings(const json::Dict& render_settings) {
		RenderSettings settings;
		settings.width = render_settings.at("width"s).AsDouble();
		settings.height = render_settings.at("height"s).AsDouble();
		settings.padding = render_settings.at("padding"s).AsDouble();
		settings.line_width = render_settings.at("line_width"s).AsDouble();
		settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();
		settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
		settings.bus_label_offset = ReadOffset(render_settings.at("bus_label_offset"s).AsArray());
		settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
		settings.stop_label_offset = ReadOffset(render_settings.at("stop_label_offset"s).AsArray());
		settings.underlayer_color = ReadColor(render_settings.at("underlayer_color"s));
		settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
		settings.color_palette = ReadColorPalette(render_settings.at("color_palette"s).AsArray());

		return settings;
	}

	svg::Point JsonReader::ReadOffset(const json::Array& offset) {
		return svg::Point(offset[0].AsDouble(), offset[1].AsDouble());
	}

	svg::Color JsonReader::ReadColor(const json::Node& color) {
		if (color.IsString()) {
			return color.AsString();
		}
		else if (color.IsArray()) {
			const auto& color_array = color.AsArray();
			if (color_array.size() == 3) {
				return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
			}
			else if (color_array.size() == 4) {
				return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
			}
		}
		assert(false);
	}

	std::vector<svg::Color> JsonReader::ReadColorPalette(const json::Array& palette) {
		std::vector<svg::Color> colors;
		colors.reserve(palette.size());
		for (const auto& color : palette) {
			colors.push_back(ReadColor(color));
		}
		return colors;
	}

	// ---------------------- Output ----------------------

	namespace {

		struct StatResultPrinter {
			json::Dict& dict;

			void operator()(std::monostate) const {
				dict.emplace("error_message"s, "not found"s);
			}

			void operator()(tc::BusInfo bus_info) const {
				dict.emplace("curvature"s, bus_info.curvature);
				dict.emplace("route_length"s, bus_info.length);
				dict.emplace("stop_count"s, bus_info.stops);
				dict.emplace("unique_stop_count"s, bus_info.unique_stops);
			}

			void operator()(tc::StopInfo stop_info) const {
				json::Array json_array;
				json_array.reserve(stop_info.buses.size());

				for (auto& bus : stop_info.buses) {
					json_array.push_back(std::move(bus));
				}

				dict.emplace("buses"s, std::move(json_array));
			}

			void operator()(std::string map_info) const {
				dict.emplace("map"s, std::move(map_info));
			}
		};

	} // namespace

	JsonWriter::JsonWriter(std::ostream& output) 
		: output_(output) {
	}

	void JsonWriter::Write(const std::vector<StatRequestResult>& results) {
		json::Array json_array;
		json_array.reserve(results.size());

		for (const auto& result : results) {
			json::Dict json_dict;
			json_dict.emplace("request_id"s, result.request_id);
			std::visit(StatResultPrinter{ json_dict }, result.result);
			json_array.push_back(std::move(json_dict));
		}

		json::Print(json::Document(std::move(json_array)), output_);
	}

} // namespace tc

#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "geo.h"
#include "json.h"
#include "map_renderer.h"

namespace io {

	// ---------------------- Input ----------------------

	using RequestId = int;

	struct BaseRequestStop {
		std::string name;
		geo::Coordinates coordinates;
		std::vector<std::pair<std::string, uint32_t>> distances;
	};

	struct BaseRequestBus {
		std::string name;
		bool ring = false;
		std::vector<std::string> stops;
	};

	struct StatRequest {
		enum class Type {
			BUS,
			STOP,
			MAP
		};

		RequestId id = 0;
		Type type = Type::BUS;
		std::string name;
	};

	struct Input {
		std::vector<BaseRequestStop> stops;
		std::vector<BaseRequestBus> buses;
		std::vector<StatRequest> stat_requests;
		RenderSettings render_settings;
	};

	class JsonReader {
	public:
		JsonReader(std::istream& input);
		Input Read();

	private:
		std::istream& input_;

		static BaseRequestBus ReadBaseRequestBus(const json::Dict& base_request);
		static BaseRequestStop ReadBaseRequestStop(const json::Dict& base_request);
		static StatRequest ReadStatRequest(const json::Dict& stat_request);
		static RenderSettings ReadRenderSettings(const json::Dict& render_settings);

		static svg::Point ReadOffset(const json::Array& offset);
		static svg::Color ReadColor(const json::Node& color);
		static std::vector<svg::Color> ReadColorPalette(const json::Array& palette);

	};

	// ---------------------- Output ----------------------

	struct StatRequestResult {
		RequestId request_id = 0;
		std::variant<std::monostate, tc::BusInfo, tc::StopInfo, std::string> result;
	};

	class JsonWriter {
	public:
		JsonWriter(std::ostream& output);
		void Write(const std::vector<StatRequestResult>& results);

	private:
		std::ostream& output_;
	};

} // namespace tc
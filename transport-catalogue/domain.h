#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace tc {

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		bool ring = false;
		std::vector<const Stop*> stops;
	};

	struct BusInfo {
		int stops = 0;
		int unique_stops = 0;
		double length = 0.0;
		double curvature = 0.0;
	};

	struct StopInfo {
		std::vector<std::string> buses;
	};

} // namespace tc

#pragma once

#include <istream>
#include <string>
#include <utility>
#include <vector>

#include "transport_catalogue.h"

namespace tc {
	
	namespace input {

		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
			std::vector<std::pair<std::string, uint32_t>> distances;
		};

		struct Bus {
			std::string name;
			bool ring;
			std::vector<std::string> stops;
		};

		struct Queries {
			std::vector<Stop> stops;
			std::vector<Bus> buses;
		};

		class Reader {
		public:
			Reader(std::istream& input);
			Queries ReadQueries(size_t count);

		private:
			std::istream& input_;

			Stop ReadStop();
			Bus ReadBus();

			static std::pair<std::string, uint32_t> ParseDistance(std::string&& distance_str);
		};

	}

}

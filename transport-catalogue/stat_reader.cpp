#include <utility>

#include "stat_reader.h"

namespace tc::output {

	Reader::Reader(std::istream& input) : input_(input) {
	}

	std::vector<Query> Reader::ReadQueries(size_t count) {
		using namespace std::string_literals;

		std::vector<Query> queries;
		queries.reserve(count);

		for (size_t i = 0; i < count; ++i) {
			std::string query_type;
			input_ >> query_type;
			if (query_type == "Bus"s) {
				std::string bus;
				std::getline(input_, bus);
				queries.push_back({ Query::Type::BUS, bus.substr(1) });
			}
			else { // query_type == "Stop"s
				std::string stop;
				std::getline(input_, stop);
				queries.push_back({ Query::Type::STOP, stop.substr(1) });
			}
		}

		return queries;
	}
}

std::ostream& tc::operator<<(std::ostream& output, const transport::StopInfo& info) {
	using namespace std::string_literals;

	output << "Stop "s << info.name << ": "s;
	if (!info.buses.has_value()) {
		output << "not found"s;
		return output;
	}
	const auto& buses = *info.buses;
	if (buses.size() == 0) {
		output << "no buses"s;
		return output;
	}

	output << "buses "s;
	for (const auto& bus : buses) {
		output << bus << ' ';
	}

	return output;
}

std::ostream& tc::operator<<(std::ostream& output, const transport::BusInfo& info) {
	using namespace std::string_literals;

	output << "Bus "s << info.name << ": "s;
	if (!info.stats.has_value()) {
		output << "not found"s;
		return output;
	}

	const auto& stats = *info.stats;
	output << stats.stops << " stops on route, "s 
		   << stats.unique_stops << " unique stops, "s 
		   << stats.length << " route length, "s
		   << stats.curvature << " curvature"s;

	return output;
}

#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
	using namespace tc;

	transport::Catalogue transport_catalogue;
	
	// Input queries
	int queries_count;
	std::cin >> queries_count;

	input::Reader input_reader(std::cin);
	const auto input_queries = input_reader.ReadQueries(queries_count);
	for (const auto& stop : input_queries.stops) {
		transport_catalogue.AddStop(stop.name, stop.coordinates);
	}
	for (const auto& stop : input_queries.stops) {
		for (const auto& [to, distance] : stop.distances) {
			transport_catalogue.AddDistance(stop.name, to, distance);
		}
	}
	for (const auto& bus : input_queries.buses) {
		transport_catalogue.AddBus(bus.name, bus.ring, bus.stops);
	}

	// Stat queries
	std::cin >> queries_count;

	output::Reader stat_reader(std::cin);
	const auto stat_queries = stat_reader.ReadQueries(queries_count);
	for (const auto& query : stat_queries) {
		if (query.type == output::Query::Type::BUS) {
			const auto bus_info = transport_catalogue.GetBusInfo(query.name);
			std::cout << bus_info << std::endl;
		}
		else {
			const auto stop_info = transport_catalogue.GetStopInfo(query.name);
			std::cout << stop_info << std::endl;
		}
	}
}

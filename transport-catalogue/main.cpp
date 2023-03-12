#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

using namespace io;
using namespace tc;

TransportCatalogue InitTransportCatalogue(std::vector<BaseRequestStop> stops, std::vector<BaseRequestBus> buses) {
	TransportCatalogue transport_catalogue;

	for (auto& stop : stops) {
		transport_catalogue.AddStop(stop.name, std::move(stop.coordinates));
	}
	for (auto& stop : stops) {
		for (auto& [to, distance] : stop.distances) {
			transport_catalogue.AddDistance(stop.name, to, distance);
		}
	}
	for (auto& bus : buses) {
		transport_catalogue.AddBus(std::move(bus.name), bus.ring, bus.stops);
	}

	return transport_catalogue;
}

std::vector<StatRequestResult> ExecuteStatRequests(const std::vector<StatRequest>& requests, const TransportCatalogue& transport_catalogue, const MapRenderer& map_renderer) {
	std::vector<StatRequestResult> results;
	results.reserve(requests.size());
		
	for (const auto& request : requests) {
		StatRequestResult result;
		result.request_id = request.id;
		
		switch (request.type)
		{
		case StatRequest::Type::BUS:
			if (auto info = transport_catalogue.GetBusInfo(request.name)) {
				result.result = *std::move(info);
			}
			break;
		case StatRequest::Type::STOP:
			if (auto info = transport_catalogue.GetStopInfo(request.name)) {
				result.result = *std::move(info);
			}
			break;
		case StatRequest::Type::MAP:
		{
			auto buses = transport_catalogue.GetBuses();
			const auto document = map_renderer.Render(std::move(buses));

			std::ostringstream sstream;
			document.Render(sstream);
			result.result = sstream.str();
		}

			break;
		default:
			assert(false);
		}

		results.push_back(std::move(result));
	}

	return results;
}

int main() {	
	JsonReader json_reader(std::cin);
	auto input = json_reader.Read();

	const auto transport_catalogue = InitTransportCatalogue(std::move(input.stops), std::move(input.buses));
	MapRenderer map_renderer(std::move(input.render_settings));

	const auto results = ExecuteStatRequests(input.stat_requests, transport_catalogue, map_renderer);

	JsonWriter json_writer(std::cout);
	json_writer.Write(results);
}

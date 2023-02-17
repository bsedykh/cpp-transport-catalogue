#include <sstream>

#include "input_reader.h"

namespace tc::input {

	Reader::Reader(std::istream& input) : input_(input) {
	}

	Queries Reader::ReadQueries(size_t count) {
		using namespace std::string_literals;

		std::vector<Stop> stops;
		std::vector<Bus> buses;

		for (size_t i = 0; i < count; ++i) {
			std::string query_type;
			input_ >> query_type;
			if (query_type == "Stop"s) {
				stops.push_back(ReadStop());
			}
			else { // query_type == "Bus"s
				buses.push_back(ReadBus());
			}
		}

		return { std::move(stops), std::move(buses) };
	}

	Stop Reader::ReadStop() {
		Stop stop;

		std::getline(input_, stop.name, ':');
		stop.name = stop.name.substr(1);

		input_ >> stop.coordinates.lat;
		
		char c;
		input_ >> c;
		
		input_ >> stop.coordinates.lng;

		input_ >> c;
		if (c != ',') {
			input_.putback(c);
			return stop;
		}

		std::string distances_str;
		std::getline(input_, distances_str);
		std::istringstream sstream(std::move(distances_str));

		std::string distance_str;
		while (std::getline(sstream, distance_str, ',')) {
			stop.distances.push_back(ParseDistance(std::move(distance_str)));
		}

		return stop;
	}

	Bus Reader::ReadBus() {
		std::string name;
		std::getline(input_, name, ':');

		std::string stops_str;
		std::getline(input_, stops_str);

		char delimiter;
		if (stops_str.find('>') != std::string::npos) {
			delimiter = '>';
		}
		else {
			delimiter = '-';
		}
		
		std::vector<std::string> stops;
		std::string stop;
		std::istringstream sstream(std::move(stops_str));
		while (std::getline(sstream, stop, delimiter)) {
			if (stop.back() == ' ') {
				stops.push_back(stop.substr(1, stop.size() - 2));
			}
			else {
				stops.push_back(stop.substr(1));
			}
		}

		return { name.substr(1), delimiter == '>', std::move(stops)};
	}

	std::pair<std::string, uint32_t> Reader::ParseDistance(std::string&& distance_str) {
		std::istringstream sstream(std::move(distance_str));

		uint32_t distance;
		sstream >> distance;

		std::string to;
		// считываем "m"
		sstream >> to;
		// считываем "to"
		sstream >> to;
		// считываем имя остановки
		std::getline(sstream, to);

		return { to.substr(1), distance};
	}

}

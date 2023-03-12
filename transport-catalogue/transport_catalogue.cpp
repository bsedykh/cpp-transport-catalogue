#include <cassert>
#include <functional>
#include <unordered_set>

#include "transport_catalogue.h"

namespace tc {

	bool TransportCatalogue::SetOfBusesCmp::operator() (const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}

	size_t TransportCatalogue::StopsHasher::operator() (const std::pair<const Stop*, const Stop*>& stops) const {
		std::hash<const Stop*> hasher;
		return hasher(stops.first) + 37 * hasher(stops.second);
	}

	void TransportCatalogue::AddStop(std::string name, geo::Coordinates coordinates) {
		Stop stop{ std::move(name), std::move(coordinates) };
		stops_.push_back(std::move(stop));
		name_to_stop_.emplace(stops_.back().name, &stops_.back());
		stop_to_buses_.emplace(&stops_.back(), SetOfBuses());
	}

	void TransportCatalogue::AddDistance(const std::string& from, const std::string& to, uint32_t distance) {
		auto stop_from = FindStop(from);
		auto stop_to = FindStop(to);
		std::pair<const Stop*, const Stop*> key(stop_from, stop_to);
		stops_to_distance_.emplace(key, distance);
	}

	void TransportCatalogue::AddBus(std::string name, bool ring, const std::vector<std::string>& stop_names) {
		assert(stop_names.size() > 1);
		Bus bus{ std::move(name), ring, {} };

		std::vector<const Stop*> stops;
		stops.reserve(stop_names.size());
		for (const auto& stop_name : stop_names) {
			auto stop = FindStop(stop_name);
			stops.push_back(stop);
		}
		bus.stops = std::move(stops);

		buses_.push_back(std::move(bus));
		name_to_bus_.emplace(buses_.back().name, &buses_.back());
		AddBusToStops(buses_.back());
	}

	std::vector<const Bus*> TransportCatalogue::GetBuses() const {
		std::vector<const Bus*> buses;
		buses.reserve(buses_.size());

		for (const auto& bus : buses_) {
			buses.push_back(&bus);
		
		}

		return buses;
	}

	std::optional<StopInfo> TransportCatalogue::GetStopInfo(const std::string& name) const {
		auto stop = FindStop(name);
		if (!stop) {
			return std::nullopt;
		}

		StopInfo stop_info;

		const auto& set_of_buses = stop_to_buses_.at(stop);
		std::vector<std::string> buses;
		buses.reserve(set_of_buses.size());
		for (auto bus : set_of_buses) {
			buses.push_back(bus->name);
		}
		stop_info.buses = std::move(buses);

		return stop_info;
	}

	std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string& name) const {
		auto bus = FindBus(name);
		if (!bus) {
			return std::nullopt;
		}

		auto& stops = bus->stops;

		size_t stops_num = bus->ring ? stops.size() : stops.size() * 2 - 1;
		std::unordered_set<const Stop*> unique_stops;
		double fact_length = 0.0;
		double geo_length = 0.0;

		for (size_t i = 0; i < stops.size() - 1; ++i) {
			fact_length += ComputeDistance(stops[i], stops[i + 1], DistanceType::FACT);
			geo_length += ComputeDistance(stops[i], stops[i + 1], DistanceType::GEO);
			unique_stops.insert(stops[i]);
		}
		unique_stops.insert(stops[stops.size() - 1]);

		if (!bus->ring) {
			for (size_t i = stops.size() - 1; i > 0; --i) {
				fact_length += ComputeDistance(stops[i], stops[i - 1], DistanceType::FACT);
			}
			geo_length *= 2.0;
		}

		assert(geo_length > 0.0);

		return BusInfo{ static_cast<int>(stops_num), static_cast<int>(unique_stops.size()), fact_length, fact_length / geo_length };
	}

	const Stop* TransportCatalogue::FindStop(const std::string& name) const {
		const auto it = name_to_stop_.find(name);
		if (it != name_to_stop_.end()) {
			return it->second;
		}
		else {
			return nullptr;
		}
	}

	const Bus* TransportCatalogue::FindBus(const std::string& name) const {
		const auto it = name_to_bus_.find(name);
		if (it != name_to_bus_.end()) {
			return it->second;
		}
		else {
			return nullptr;
		}
	}

	void TransportCatalogue::AddBusToStops(const Bus& bus) {
		for (auto stop : bus.stops) {
			stop_to_buses_[stop].insert(&bus);
		}
	}

	double TransportCatalogue::ComputeDistance(const Stop* from, const Stop* to, DistanceType type) const {
		if (type == DistanceType::FACT) {
			auto it = stops_to_distance_.find({ from, to });
			if (it != stops_to_distance_.end()) {
				return it->second;
			}

			it = stops_to_distance_.find({ to, from });
			if (it != stops_to_distance_.end()) {
				return it->second;
			}
		}

		return geo::ComputeDistance(from->coordinates, to->coordinates);
	}

} // namespace tc

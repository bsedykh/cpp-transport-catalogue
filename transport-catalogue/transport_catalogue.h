#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "domain.h"

namespace tc {

		class TransportCatalogue {
			struct StopsHasher {
				size_t operator() (const std::pair<const Stop*, const Stop*>& stops) const;
			};

			struct SetOfBusesCmp {
				bool operator() (const Bus* lhs, const Bus* rhs) const;
			};
			using SetOfBuses = std::set<const Bus*, SetOfBusesCmp>;

			enum class DistanceType {
				GEO,
				FACT
			};

		public:
			void AddStop(std::string name, geo::Coordinates coordinates);
			void AddDistance(const std::string& from, const std::string& to, uint32_t distance);
			void AddBus(std::string name, bool ring, const std::vector<std::string>& stop_names);
			
			std::vector<const Bus*> GetBuses() const;
			std::optional<StopInfo> GetStopInfo(const std::string& name) const;
			std::optional<BusInfo> GetBusInfo(const std::string& name) const;			

		private:
			std::deque<Stop> stops_;
			std::deque<Bus> buses_;

			std::unordered_map<std::string_view, const Stop*> name_to_stop_;
			std::unordered_map<std::string_view, const Bus*> name_to_bus_;
			std::unordered_map<const Stop*, SetOfBuses> stop_to_buses_;
			std::unordered_map<std::pair<const Stop*, const Stop*>, uint32_t, StopsHasher> stops_to_distance_;

			const Stop* FindStop(const std::string& name) const;
			const Bus* FindBus(const std::string& name) const;
			void AddBusToStops(const Bus& bus);
			double ComputeDistance(const Stop* from, const Stop* to, DistanceType type) const;
		};

} // namespace tc
